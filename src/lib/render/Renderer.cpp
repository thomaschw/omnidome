/* Copyright (c) 2014-2015 "Omnidome" by cr8tr
 * Dome Mapping Projection Software (http://omnido.me).
 * Omnidome was created by Michael Winkelmann aka Wilston Oreo (@WilstonOreo)
 *
 * This file is part of Omnidome.
 *
 * Omnidome is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <omni/render/Renderer.h>

#include <QPainter>

#include <fstream>

#include <omni/util.h>
#include <omni/visual/util.h>
#include <omni/visual/Session.h>
#include <omni/visual/Tuning.h>
#include <omni/proj/ScreenSetup.h>

namespace omni {
  namespace render {
    Renderer::Renderer(Session const& _session) :
      session_(_session)
    {}

    std::map<QScreen const *, QImage>Renderer::stitchScreens(
      std::vector<const proj::Tuning *>const& _tunings) const
    {
      // Get all tunings for each screen
      std::map<QScreen const *,
               std::vector<proj::Tuning const *> > _screenToTunings;

      for (auto& _tuning : _tunings)
      {
        auto *_screen = _tuning->screen();

        if (_screenToTunings.count(_screen) == 0)
        {
          _screenToTunings[_screen] = std::vector<proj::Tuning const *>(
            { _tuning });
        }
        else
        {
          _screenToTunings[_screen].push_back(_tuning);
        }
      }

      /// Render screen images from tunings
      std::map<QScreen const *, QImage> _result;

      for (auto& _screenTunings : _screenToTunings)
      {
        auto *_screen  = _screenTunings.first;
        auto& _tunings = _screenTunings.second;

        auto   _screenRect = session_.screenSetup().screenGeometry(_screen);
        QImage _image(_screenRect.width(),
                      _screenRect.height() * 3, QImage::Format_RGB32); // Overall
                                                                       // screen
                                                                       // image
        QPainter _p(&_image);

        for (auto& _tuning : _tunings)
        {
          auto _rect = _tuning->contentGeometry();
          proj::Calibration _calib(*_tuning,
                                   session_.exportSettings().outputMode());
          auto && _tuningImage = _calib.toImage();
          _p.drawImage(_rect.x(), 0, _tuningImage);
        }
        _p.end();
        _result[_screen] = _image;
      }
      return _result;
    }

    void Renderer::renderToFile(QString const& _filename)
    {
      auto& _settings = session_.exportSettings();

      std::vector<proj::Tuning const *> _tunings;

      QString _rawName = util::removeFileExt(_filename);

      // Get all tunings
      for (auto& _tuning : session_.tunings())
      {
        if (!_tuning->hasScreen() &&
            _settings.excludeUnassignedProjectors()) continue;
        _tunings.push_back(_tuning.get());
      }

      switch (_settings.separationMode())
      {
      /// Merge all together
      case SeparationMode::NONE:
      {
        auto _desktopRect = proj::ScreenSetup::desktopRect();
        auto && _screens = stitchScreens(_tunings);
        QImage _stitchedImage(_desktopRect.width(),
                              _desktopRect.height() * 2,
                              QImage::Format_RGB32);
        QPainter _p(&_stitchedImage);

        for (auto& _screenImage : _screens)
        {
          auto *_screen = _screenImage.first;
          auto& _image  = _screenImage.second;
          _p.drawImage(
            session_.screenSetup().screenGeometry(
              _screen).topLeft() - _desktopRect.topLeft(),
            _image);
        }
        _stitchedImage.save(_filename);
        break;
      }

      /// A file for each screen
      case SeparationMode::SCREENS:
      {
        auto && _screens = stitchScreens(_tunings);

        for (auto& _screenImage : _screens)
        {
          auto *_screen      = _screenImage.first;
          auto& _image       = _screenImage.second;
          auto  _desktopRect = proj::ScreenSetup::desktopRect();
          auto && _screenRect = session_.screenSetup().screenGeometry(_screen);
          QString _screenDesc(QString("_%1_%2_%3x%4.png").
                              arg(_screenRect.left()).
                              arg(_screenRect.top()).
                              arg(_screenRect.width()).
                              arg(_screenRect.height()));
          _image.save(_rawName + _screenDesc);
        }
        break;
      }

      case SeparationMode::PROJECTORS:
      {
        int _tuningIndex = 0;

        for (auto& _tuning : _tunings)
        {
          proj::Calibration _calib(*_tuning,
                                   session_.exportSettings().outputMode());
          auto && _image = _calib.toImage();
          _image.save(_rawName + QString("_%1.png").arg(_tuningIndex + 1));
        }
        break;
      }
      }
    }

    void Renderer::renderOmniCalibration(QString const& _filename) {
      auto& _settings = session_.exportSettings();

      std::vector<proj::Tuning const *> _tunings;

      // Get all tunings
      for (auto& _tuning : session_.tunings())
      {
        if (!_tuning->hasScreen() &&
            _settings.excludeUnassignedProjectors()) continue;
        _tunings.push_back(_tuning.get());
      }

      std::ofstream _file(_filename.toStdString(),std::ofstream::out);

      // Write header
      std::string _header("OMNIC_v1.0.0.0");
      _header.resize(80);
      _file.write(_header.c_str(),80);

      _file << uint32_t(_settings.outputMode());
      _file << uint32_t(_tunings.size());

      auto _writeRect = [&](QRect const& _r) {
        _file << int32_t(_r.left()); // offset_x
        _file << int32_t(_r.top()); // offset_y
        _file << uint32_t(_r.width()); // width
        _file << uint32_t(_r.height()); // height
      };

      auto _writeChannelCorrection = [&](proj::ChannelCorrection const& _c) {
        _file << float(_c.gamma());
        _file << float(_c.brightness());
        _file << float(_c.contrast());
        _file << float(_c.multiplier());
      };

      for (auto* _tuning : _tunings) {
          proj::Calibration _calib(*_tuning,_settings.outputMode());

          _file << uint32_t(_calib.virtualScreen());
          _writeRect(_calib.screenGeometry());
          _writeRect(_calib.contentGeometry());

          _writeChannelCorrection(_calib.colorCorrection().red());
          _writeChannelCorrection(_calib.colorCorrection().green());
          _writeChannelCorrection(_calib.colorCorrection().blue());
          _writeChannelCorrection(_calib.colorCorrection().all());

          /// Write render buffer
          uint32_t _w = _calib.renderSize().width();
          uint32_t _h = _calib.renderSize().height();
          _file << _w;
          _file << _h;
          _file.write((const char*)_calib.buffer().ptr(),sizeof(RGBAFloat)*_w*_h);

          /// Brightness correction, not used currently
          _file << uint32_t(0) << uint32_t(0);
      }
    }
  }
}

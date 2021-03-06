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

#ifndef OMNI_UI_CANVASPARAMETERS_H_
#define OMNI_UI_CANVASPARAMETERS_H_

#include <QComboBox>
#include <omni/canvas/Interface.h>
#include <omni/ui/ParameterWidget.h>
#include <omni/ui/mixin/DataModel.h>

namespace omni {
  namespace ui {
    /**@brief Parameter widget for getting and setting canvas parameters
    **/
    class CanvasParameters :
      public ParameterWidget,
      public mixin::UnsharedDataModel<canvas::Interface>{
        Q_OBJECT
        OMNI_UI_UNSHARED_DATAMODEL(canvas::Interface)
      public:
        CanvasParameters(QWidget *_parent = nullptr);
        virtual ~CanvasParameters();

      public slots:
        // Set data model with scale and unit
        void setDataModel(canvas::Interface*);

       /**@brief Set flag if values are rescaled
          @param _rescale Boolean which tells if values are rescaled
        */
        virtual void     setRescaleValues(bool _rescale);

        /**@brief Set slider ranges
           @param _factor Scale factor
         **/
        virtual void     setScale(float _factor);

        /// Set slider units
        virtual void     setUnit(QString const&);

      signals:
        void         dataModelChanged();

      protected:
        virtual void dataToFrontend();

        /// Return true if data has changed by front end
        virtual bool frontendToData();

      private:
        /// Transform widget
        QUniquePtr<omni::ui::AffineTransform> transform_;
        QUniquePtr<QComboBox> boxViewMode_;
    };
  }
}


#endif /* OMNI_UI_CANVASPARAMETERS_H_ */

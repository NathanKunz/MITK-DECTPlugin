/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkDualEnergyCtConversionPreferencePage_h
#define QmitkDualEnergyCtConversionPreferencePage_h

#include <berryIQtPreferencePage.h>
#include <org_mitk_gui_qt_dualenergyctconversion_Export.h>
#include <QString>

class QWidget;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QCheckBox;

/**
 * @brief      GUI class for the qmitk dual energy ct conversion preference page.
 */
class QmitkDualEnergyCtConversionPreferencePage : public QObject, public berry::IQtPreferencePage
{
	Q_OBJECT
	Q_INTERFACES(berry::IPreferencePage)

public:

    QmitkDualEnergyCtConversionPreferencePage();
	~QmitkDualEnergyCtConversionPreferencePage() override;


    /**
     * \brief Called by framework to initialise this preference page, but currently does nothing.
     * \param workbench The workbench.
     */
    void Init(berry::IWorkbench::Pointer workbench) override;

    /**
     * \brief Called by framework to create the GUI, and connect signals and slots.
     * \param widget The Qt widget that acts as parent to all GUI components, as this class itself is not derived from QWidget.
     */
    void CreateQtControl(QWidget* widget) override;

    /**
     * \brief Required by framework to get hold of the GUI.
     * \return QWidget* the top most QWidget for the GUI.
     */
    QWidget* GetQtControl() const override;

    /**
     * \see IPreferencePage::PerformOk
     */
    bool PerformOk() override;

    /**
     * \see IPreferencePage::PerformCancel
     */
    void PerformCancel() override;

    /**
     * \see IPreferencePage::Update
     */
    void Update() override;

protected:
    QWidget* m_MainControl;
    berry::IPreferences::Pointer m_DualEnergyConversionPreferenceNode;

    QLineEdit* m_PathEdit;
    QPushButton* m_PathSelect;
    QRadioButton* m_RadioOverwrite;
    QRadioButton* m_RadioAppend;
    QCheckBox* m_EnableExternalCheckBox;

protected slots:
	/**
     * @brief      Function called once the gui button m_PathSelect is pressed, 
     *             opens file dialog and writes the selected file into m_PathEdit
     */
    void PathSelectButtonPushed();


    /**
     * @brief      Funktion called when the checkbox m_EnableExternalCheckBox is pressed.
     *             Disable or enable all other member gui object, depending on the state of the checkbox.
     *
     * @param[in]  state  The state
     */
    void CheckboxChanged(int state);
	
};

#endif

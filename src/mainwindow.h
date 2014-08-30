/*
  This file is part of Shuriken Beat Slicer.

  Copyright (C) 2014 Andrew M Taylor <a.m.taylor303@gmail.com>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program. If not, see <https://www.gnu.org/licenses/>
  or write to the Free Software Foundation, Inc., 51 Franklin Street,
  Fifth Floor, Boston, MA  02110-1301, USA.

*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QUndoStack>
#include "JuceHeader.h"
#include "samplebuffer.h"
#include "optionsdialog.h"
#include "audiofilehandler.h"
#include "sampleraudiosource.h"
#include "rubberbandaudiosource.h"
#include "slicepointitem.h"
#include "audioanalyser.h"
#include "waveformitem.h"
#include "helpform.h"
#include "exportdialog.h"


namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    friend class DeleteWaveformItemCommand;
    friend class SliceCommand;
    friend class JoinCommand;
    friend class SplitCommand;
    friend class ApplyTimeStretchCommand;

public:
    MainWindow( QWidget* parent = NULL );
    ~MainWindow();

    void connectWaveformToMainWindow( const SharedWaveformItem item );

    void openProject( const QString filePath );

protected:
    void changeEvent( QEvent* event );
    void closeEvent( QCloseEvent* event );
    void keyPressEvent( QKeyEvent* event );

private:
    void initialiseAudio();
    void setUpSampler( const SharedSampleBuffer sampleBuffer, const SharedSampleHeader sampleHeader );
    void tearDownSampler();

    void setupUI();
    void enableUI();
    void disableUI();

    void updateSnapLoopMarkersComboBox();

    void getDetectionSettings( AudioAnalyser::DetectionSettings& settings );
    void getSampleRanges( QList<SharedSampleRange>& sampleRangeList );

    void closeProject();

    void exportAs( const QString tempDirPath,
                   const QString outputDirPath,
                   const QString samplesDirPath,
                   const QString fileName,
                   const int exportType,
                   const int sndFileFormat,
                   const int outputSampleRate,
                   const int numSamplesToExport );

    void saveProject( const QString filePath );

    void saveProjectDialog();
    void openProjectDialog();
    void importAudioFileDialog();
    void exportAsDialog();

    Ui::MainWindow* mUI; // "Go to slot..." in Qt Designer won't work if this is changed to ScopedPointer<Ui::MainWindow>

    enum LengthUnit { UNITS_BARS, UNITS_BEATS };

    ScopedPointer<OptionsDialog> mOptionsDialog;
    ScopedPointer<HelpForm> mHelpForm;
    ScopedPointer<ExportDialog> mExportDialog;

    AudioDeviceManager mDeviceManager;
    AudioFileHandler mFileHandler;

    SharedSampleBuffer mCurrentSampleBuffer;
    SharedSampleHeader mCurrentSampleHeader;
    QList<SharedSampleRange> mSampleRangeList;
    QList<SharedSampleRange> mLoopSampleRangeList;

    ScopedPointer<SamplerAudioSource> mSamplerAudioSource;
    ScopedPointer<RubberbandAudioSource> mRubberbandAudioSource;
    AudioSourcePlayer mAudioSourcePlayer;

    bool mIsAudioInitialised;

    QString mLastOpenedImportDir;
    QString mLastOpenedProjDir;
    QString mCurrentProjectFilePath;

    QUndoStack mUndoStack;

    qreal mAppliedBPM;

    bool mIsProjectOpen;

public slots:
    void reorderSampleRangeList( QList<int> oldOrderPositions, const int numPlacesMoved );

private slots:
    void on_comboBox_SnapLoopMarkers_currentIndexChanged( const int index );
    void on_checkBox_LoopMarkers_clicked( const bool isChecked );
    void on_pushButton_TimestretchOptions_clicked();
    void on_actionAudition_triggered();
    void on_actionSelect_triggered();
    void on_actionMove_triggered();
    void on_pushButton_Apply_clicked();
    void on_actionZoom_Original_triggered();
    void on_actionZoom_Out_triggered();
    void on_actionZoom_In_triggered();
    void on_pushButton_Loop_clicked( const bool isChecked );
    void on_pushButton_PlayStop_clicked();
    void on_checkBox_PitchCorrection_toggled( const bool isChecked );
    void on_checkBox_TimeStretch_toggled( const bool isChecked );
    void on_doubleSpinBox_NewBPM_valueChanged( const double newBPM );
    void on_doubleSpinBox_OriginalBPM_valueChanged( const double originalBPM );
    void on_pushButton_CalcBPM_clicked();
    void on_pushButton_FindBeats_clicked();
    void on_horizontalSlider_Threshold_valueChanged( const int value );
    void on_pushButton_FindOnsets_clicked();
    void on_pushButton_Slice_clicked();
    void on_actionNormalise_triggered();
    void on_actionApply_Gain_Ramp_triggered();
    void on_actionApply_Gain_triggered();
    void on_actionSelect_None_triggered();
    void on_actionSelect_All_triggered();
    void on_actionSplit_triggered();
    void on_actionJoin_triggered();
    void on_actionAbout_triggered();
    void on_actionHelp_triggered();
    void on_actionOptions_triggered();
    void on_actionEnvelope_triggered();
    void on_actionReverse_triggered();
    void on_actionDelete_triggered();
    void on_actionAdd_Slice_Point_triggered();
    void on_actionRedo_triggered();
    void on_actionUndo_triggered();
    void on_actionQuit_triggered();
    void on_actionExport_As_triggered();
    void on_actionImport_Audio_File_triggered();
    void on_actionClose_Project_triggered();
    void on_actionSave_As_triggered();
    void on_actionSave_Project_triggered();
    void on_actionOpen_Project_triggered();


    void recordWaveformItemMove( QList<int> oldOrderPositions, const int numPlacesMoved );

    void recordSlicePointItemMove( const SharedSlicePointItem slicePointItem,
                                   const int oldFrameNum,
                                   const int newFrameNum );

    void playSampleRange( const int waveformItemStartFrame,
                          const int waveformItemNumFrames,
                          const QPointF mouseScenePos );

    void stopPlayback();

    void resetPlayStopButtonIcon();

    void setLoopSampleRanges();

    void disableZoomIn();
    void disableZoomOut();

    void enableRealtimeControls( const bool isEnabled );
    void resetSampler();

    void enableEditActions();
    void enableSaveAction();

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR( MainWindow );
};

#endif // MAINWINDOW_H

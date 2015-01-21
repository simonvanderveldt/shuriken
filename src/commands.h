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

#ifndef COMMANDS_H
#define COMMANDS_H

#include <QUndoCommand>
#include <QPushButton>
#include <QAction>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include "wavegraphicsview.h"
#include "slicepointitem.h"
#include "mainwindow.h"


class AddSlicePointItemCommand : public QUndoCommand
{
public:
    AddSlicePointItemCommand( int frameNum,
                              bool canBeMovedPastOtherSlicePoints,
                              WaveGraphicsView* graphicsView,
                              QPushButton* sliceButton = NULL,
                              QUndoCommand* parent = NULL );

    void undo();
    void redo();

private:
    WaveGraphicsView* const m_graphicsView;
    QPushButton* const m_sliceButton;
    SharedSlicePointItem m_slicePointItem;
    bool m_isFirstRedoCall;
};



class MoveSlicePointItemCommand : public QUndoCommand
{
public:
    MoveSlicePointItemCommand( SharedSlicePointItem slicePoint,
                               int oldFrameNum,
                               WaveGraphicsView* graphicsView,
                               QUndoCommand* parent = NULL );

    void undo();
    void redo();

private:
    const SharedSlicePointItem m_slicePointItem;
    const int m_oldFrameNum;
    const int m_newFrameNum;
    WaveGraphicsView* const m_graphicsView;
    bool m_isFirstRedoCall;
};



class DeleteSlicePointItemCommand : public QUndoCommand
{
public:
    DeleteSlicePointItemCommand( SharedSlicePointItem slicePoint,
                                 WaveGraphicsView* graphicsView,
                                 QPushButton* sliceButton = NULL,
                                 QUndoCommand* parent = NULL );

    void undo();
    void redo();

private:
    const SharedSlicePointItem m_slicePointItem;
    WaveGraphicsView* const m_graphicsView;
    QPushButton* const m_sliceButton;
};



class SliceCommand : public QUndoCommand
{
public:
    SliceCommand( MainWindow* mainWindow,
                  WaveGraphicsView* graphicsView,
                  QPushButton* sliceButton,
                  QPushButton* findOnsetsButton,
                  QPushButton* findBeatsButton,
                  QAction* addSlicePointAction,
                  QAction* selectMoveItemsAction,
                  QAction* auditionItemsAction,
                  QAction* selectiveTimeStretchAction,
                  QUndoCommand* parent = NULL );

    void undo();
    void redo();

private:
    MainWindow* const m_mainWindow;
    WaveGraphicsView* const m_graphicsView;
    QPushButton* const m_sliceButton;
    QPushButton* const m_findOnsetsButton;
    QPushButton* const m_findBeatsButton;
    QAction* const m_addSlicePointAction;
    QAction* const m_selectMoveItemsAction;
    QAction* const m_auditionItemsAction;
    QAction* const m_selectiveTimeStretchAction;
};



class UnsliceCommand : public QUndoCommand
{
public:
    UnsliceCommand( MainWindow* mainWindow,
                    WaveGraphicsView* graphicsView,
                    QPushButton* sliceButton,
                    QPushButton* findOnsetsButton,
                    QPushButton* findBeatsButton,
                    QAction* addSlicePointAction,
                    QAction* selectMoveItemsAction,
                    QAction* auditionItemsAction,
                    QAction* selectiveTimeStretchAction,
                    QUndoCommand* parent = NULL );

    void undo();
    void redo();

private:
    MainWindow* const m_mainWindow;
    WaveGraphicsView* const m_graphicsView;
    QPushButton* const m_sliceButton;
    QPushButton* const m_findOnsetsButton;
    QPushButton* const m_findBeatsButton;
    QAction* const m_addSlicePointAction;
    QAction* const m_selectMoveItemsAction;
    QAction* const m_auditionItemsAction;
    QAction* const m_selectiveTimeStretchAction;
};



class EnableSelectiveTSCommand : public QUndoCommand
{
public:
    EnableSelectiveTSCommand( WaveGraphicsView* graphicsView,
                              QPushButton* sliceButton,
                              QAction* addSlicePointAction,
                              QAction* selectMoveItemsAction,
                              QAction* multiSelectItemsAction,
                              QAction* auditionItemsAction,
                              QAction* selectiveTimeStretchAction,
                              const QList<SharedSampleBuffer> sampleBufferList,
                              QUndoCommand* parent = NULL );

    void undo();
    void redo();

private:
    WaveGraphicsView* const m_graphicsView;
    QPushButton* const m_sliceButton;
    QAction* const m_addSlicePointAction;
    QAction* const m_selectMoveItemsAction;
    QAction* const m_multiSelectItemsAction;
    QAction* const m_auditionItemsAction;
    QAction* const m_selectiveTimeStretchAction;
    const QList<SharedSampleBuffer> m_sampleBufferList;
};



class DisableSelectiveTSCommand : public QUndoCommand
{
public:
    DisableSelectiveTSCommand( WaveGraphicsView* graphicsView,
                               QPushButton* sliceButton,
                               QAction* addSlicePointAction,
                               QAction* selectMoveItemsAction,
                               QAction* multiSelectItemsAction,
                               QAction* auditionItemsAction,
                               QAction* selectiveTimeStretchAction,
                               const QList<SharedSampleBuffer> sampleBufferList,
                               QUndoCommand* parent = NULL );

    void undo();
    void redo();

private:
    WaveGraphicsView* const m_graphicsView;
    QPushButton* const m_sliceButton;
    QAction* const m_addSlicePointAction;
    QAction* const m_selectMoveItemsAction;
    QAction* const m_multiSelectItemsAction;
    QAction* const m_auditionItemsAction;
    QAction* const m_selectiveTimeStretchAction;
    const QList<SharedSampleBuffer> m_sampleBufferList;
};



class MoveWaveformItemCommand : public QUndoCommand
{
public:
    MoveWaveformItemCommand( QList<int> oldOrderPositions,
                             int numPlacesMoved,
                             WaveGraphicsView* graphicsView,
                             MainWindow* mainWindow,
                             QUndoCommand* parent = NULL );

    void undo();
    void redo();

private:
    const QList<int> m_oldOrderPositions;
    const int m_numPlacesMoved;
    WaveGraphicsView* const m_graphicsView;
    MainWindow* const m_mainWindow;
    QList<int> m_newOrderPositions;
    bool m_isFirstRedoCall;
};



class DeleteWaveformItemCommand : public QUndoCommand
{
public:
    DeleteWaveformItemCommand( QList<int> orderPositions,
                               WaveGraphicsView* graphicsView,
                               MainWindow* mainWindow,
                               QPushButton* sliceButton,
                               QPushButton* findOnsetsButton,
                               QPushButton* findBeatsButton,
                               QAction* addSlicePointAction,
                               QUndoCommand* parent = NULL );

    void undo();
    void redo();

private:
    const QList<int> m_orderPositions;
    WaveGraphicsView* const m_graphicsView;
    MainWindow* const m_mainWindow;
    QPushButton* const m_sliceButton;
    QPushButton* const m_findOnsetsButton;
    QPushButton* const m_findBeatsButton;
    QAction* const m_addSlicePointAction;
    QList<SharedSampleBuffer> m_removedSampleBuffers;
    QList<SharedWaveformItem> m_removedWaveforms;
};



class ApplyGainCommand : public QUndoCommand
{
public:
    ApplyGainCommand( float gain,
                      int waveformItemOrderPos,
                      WaveGraphicsView* graphicsView,
                      int sampleRate,
                      AudioFileHandler& fileHandler,
                      QString tempDirPath,
                      QString fileBaseName,
                      QUndoCommand* parent = NULL );

    void undo();
    void redo();

private:
    const float m_gain;
    const int m_orderPos;
    WaveGraphicsView* const m_graphicsView;
    const int m_sampleRate;
    AudioFileHandler& m_fileHandler;
    const QString m_tempDirPath;
    const QString m_fileBaseName;
    QString m_filePath;
};



class ApplyGainRampCommand : public QUndoCommand
{
public:
    ApplyGainRampCommand( float startGain,
                          float endGain,
                          int waveformItemOrderPos,
                          WaveGraphicsView* graphicsView,
                          int sampleRate,
                          AudioFileHandler& fileHandler,
                          QString tempDirPath,
                          QString fileBaseName,
                          QUndoCommand* parent = NULL );

    void undo();
    void redo();

private:
    const float m_startGain;
    const float m_endGain;
    const int m_orderPos;
    WaveGraphicsView* const m_graphicsView;
    const int m_sampleRate;
    AudioFileHandler& m_fileHandler;
    const QString m_tempDirPath;
    const QString m_fileBaseName;
    QString m_filePath;
};



class NormaliseCommand : public QUndoCommand
{
public:
    NormaliseCommand( int waveformItemOrderPos,
                      WaveGraphicsView* graphicsView,
                      int sampleRate,
                      AudioFileHandler& fileHandler,
                      QString tempDirPath,
                      QString fileBaseName,
                      QUndoCommand* parent = NULL );

    void undo();
    void redo();

private:
    const int m_orderPos;
    WaveGraphicsView* const m_graphicsView;
    const int m_sampleRate;
    AudioFileHandler& m_fileHandler;
    const QString m_tempDirPath;
    const QString m_fileBaseName;
    QString m_filePath;
};



class ReverseCommand : public QUndoCommand
{
public:
    ReverseCommand( int waveformItemOrderPos,
                    WaveGraphicsView* graphicsView,
                    QUndoCommand* parent = NULL );

    void undo();
    void redo();

private:
    const int mOrderPos;
    WaveGraphicsView* const m_graphicsView;
};



class GlobalTimeStretchCommand : public QUndoCommand
{
public:
    GlobalTimeStretchCommand( MainWindow* mainWindow,
                              WaveGraphicsView* graphicsView,
                              QDoubleSpinBox* spinBoxOriginalBPM,
                              QDoubleSpinBox* spinBoxNewBPM,
                              QCheckBox* checkBoxPitchCorrection,
                              QString tempDirPath,
                              QString fileBaseName,
                              QUndoCommand* parent = NULL );

    void undo();
    void redo();

private:
    void updateSlicePoints( qreal timeRatio );
    void updateLoopMarkers( qreal timeRatio );

    MainWindow* const m_mainWindow;
    WaveGraphicsView* const m_graphicsView;
    QDoubleSpinBox* const m_spinBoxOriginalBPM;
    QDoubleSpinBox* const m_spinBoxNewBPM;
    QCheckBox* const m_checkBoxPitchCorrection;
    const qreal m_originalBPM;
    const qreal m_newBPM;
    const qreal m_prevAppliedBPM;
    const bool m_isPitchCorrectionEnabled;
    const RubberBandStretcher::Options m_options;
    const QString m_tempDirPath;
    const QString m_fileBaseName;
    QStringList m_tempFilePaths;
};



class RenderTimeStretchCommand : public QUndoCommand
{
public:
    RenderTimeStretchCommand( MainWindow* mainWindow,
                              WaveGraphicsView* graphicsView,
                              QString tempDirPath,
                              QString fileBaseName,
                              QUndoCommand* parent = NULL );

    void undo();
    void redo();

private:
    MainWindow* const m_mainWindow;
    WaveGraphicsView* const m_graphicsView;
    const RubberBandStretcher::Options m_options;
    const QString m_tempDirPath;
    const QString m_fileBaseName;
    QStringList m_tempFilePaths;
    QList<qreal> m_timeRatioList;
};



class SelectiveTimeStretchCommand : public QUndoCommand
{
public:
    SelectiveTimeStretchCommand( MainWindow* mainWindow,
                                 WaveGraphicsView* graphicsView,
                                 QList<int> orderPositions,
                                 QList<qreal> timeRatios,
                                 QList<int> midiNotes,
                                 QUndoCommand* parent = NULL );

    void undo();
    void redo();

private:
    MainWindow* const m_mainWindow;
    WaveGraphicsView* const m_graphicsView;
    const QList<int> m_orderPositions;
    const QList<qreal> m_origTimeRatios;
    const QList<qreal> m_timeRatios;
    const QList<int> m_midiNotes;
};


#endif // COMMANDS_H

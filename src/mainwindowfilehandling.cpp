/*
  This file is part of Shuriken Beat Slicer.

  Copyright (C) 2014, 2015 Andrew M Taylor <a.m.taylor303@gmail.com>

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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QDesktopWidget>
#include "commands.h"
#include "globals.h"
#include "zipper.h"
#include "messageboxes.h"
#include "textfilehandler.h"
#include "akaifilehandler.h"
#include "midifilehandler.h"
#include "confirmbpmdialog.h"
//#include <QtDebug>


//==================================================================================================
// Private:

void MainWindow::saveProject( const QString filePath, const bool isNsmSessionExport )
{
    if ( m_optionsDialog == NULL )
    {
        return;
    }

    const QString tempDirPath = m_optionsDialog->getTempDirPath();

    if ( tempDirPath.isEmpty() )
    {
        MessageBoxes::showWarningDialog( tr("Temp directory is invalid!"),
                                         tr("This operation needs to save temporary files, please change \"Temp Dir\" in options") );
        return;
    }

    QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

    const QString zipFileName = QFileInfo( filePath ).fileName();
    const QString projectName = QFileInfo( filePath ).baseName();

    QDir tempDir( tempDirPath );

    // Should not happen, but just as a precaution...
    if ( tempDir.exists( projectName ) )
    {
        tempDir.rename( projectName, projectName + ".bak" );
    }

    tempDir.mkdir( projectName );

    QDir projTempDir( tempDir.absoluteFilePath( projectName ) );

    const QString zipFilePath = tempDir.absoluteFilePath( zipFileName );

    const QString xmlFilePath = projTempDir.absoluteFilePath( "shuriken.xml" );

    bool isOkToContinue = true;
    QStringList audioFileNames;

    for ( int i = 0; i < m_sampleBufferList.size(); i++ )
    {
        const QString audioFilePath = m_fileHandler.saveAudioFile( projTempDir.absolutePath(),
                                                                   "audio" + QString::number( i ),
                                                                   m_sampleBufferList.at( i ),
                                                                   m_sampleHeader->sampleRate,
                                                                   m_sampleHeader->sampleRate,
                                                                   AudioFileHandler::SAVE_FORMAT );

        if ( ! audioFilePath.isEmpty() )
        {
            audioFileNames << QFileInfo( audioFilePath ).fileName();
        }
        else
        {
            isOkToContinue = false;
            break;
        }
    }

    if ( isOkToContinue )
    {
        TextFileHandler::ProjectSettings settings;

        settings.projectName = projectName;

        if ( m_rubberbandAudioSource != NULL )
        {
            const int startMidiNote = m_samplerAudioSource->getLowestAssignedMidiNote();

            for ( int i = 0; i < m_sampleBufferList.size(); i++ )
            {
                settings.midiNotes << startMidiNote + i;
                settings.noteTimeRatios << m_rubberbandAudioSource->getNoteTimeRatio( startMidiNote + i );
            }

            settings.originalBpm = m_ui->doubleSpinBox_OriginalBPM->value();
            settings.newBpm = m_ui->doubleSpinBox_NewBPM->value();
            settings.appliedBpm = m_appliedBPM;
        }
        else
        {
            settings.originalBpm = m_appliedBPM > 0.0 ? m_appliedBPM : m_ui->doubleSpinBox_OriginalBPM->value();
            settings.newBpm = settings.originalBpm;
            settings.appliedBpm = m_appliedBPM;
        }

        SamplerAudioSource::EnvelopeSettings envelopes;

        m_samplerAudioSource->getEnvelopeSettings( envelopes );

        settings.attackValues = envelopes.attackValues;
        settings.releaseValues = envelopes.releaseValues;
        settings.oneShotSettings = envelopes.oneShotSettings;

        settings.isTimeStretchChecked = m_ui->checkBox_TimeStretch->isChecked();
        settings.isPitchCorrectionChecked = m_ui->checkBox_PitchCorrection->isChecked();
        settings.options = m_optionsDialog->getStretcherOptions();
        settings.isJackSyncChecked = m_optionsDialog->isJackSyncEnabled();

        settings.timeSigNumerator = m_ui->comboBox_TimeSigNumerator->currentText().toInt();
        settings.timeSigDenominator = m_ui->comboBox_TimeSigDenominator->currentText().toInt();
        settings.length = m_ui->spinBox_Length->value();
        settings.units = m_ui->comboBox_Units->currentIndex();

        settings.audioFileNames = audioFileNames;

        settings.slicePointFrameNums = m_graphicsScene->getSlicePointFrameNums();

        TextFileHandler::createProjectXmlFile( xmlFilePath, settings );

        Zipper::compress( projTempDir.absolutePath(), zipFilePath );

        QFile::remove( filePath );
        QFile::copy( zipFilePath, filePath );
        QFile::remove( zipFilePath );
        File( projTempDir.absolutePath().toLocal8Bit().data() ).deleteRecursively();

        if ( m_nsmThread != NULL )
        {
            if ( m_undoStack.isClean() && ! isNsmSessionExport )
            {
                m_nsmThread->sendMessage( NsmListenerThread::MSG_IS_CLEAN );
                m_ui->actionSave_Project->setEnabled( false );
            }
        }
        else
        {
            m_currentProjectFilePath = filePath;
            addPathToRecentProjects( filePath );
        }

        if ( ! isNsmSessionExport )
        {
            m_undoStack.setClean();
        }

        QApplication::restoreOverrideCursor();
    }
    else // An error occurred while writing the audio files
    {
        QApplication::restoreOverrideCursor();
        MessageBoxes::showWarningDialog( m_fileHandler.getLastErrorTitle(), m_fileHandler.getLastErrorInfo() );
    }
}



void MainWindow::openProject( const QString filePath )
{
    if ( m_optionsDialog == NULL )
    {
        return;
    }

    const QString tempDirPath = m_optionsDialog->getTempDirPath();

    if ( tempDirPath.isEmpty() )
    {
        MessageBoxes::showWarningDialog( tr("Temp directory is invalid!"),
                                         tr("This operation needs to save temporary files, please change \"Temp Dir\" in options") );
        return;
    }

    QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

    Zipper::decompress( filePath, tempDirPath );

    const QFileInfo zipFileInfo( filePath );
    const QString projectName = zipFileInfo.baseName();

    const QDir tempDir( tempDirPath );
    QDir projTempDir = tempDir.absoluteFilePath( projectName );

    const QString xmlFilePath = projTempDir.absoluteFilePath( "shuriken.xml" );
    TextFileHandler::ProjectSettings settings;

    const bool isSuccessful = TextFileHandler::readProjectXmlFile( xmlFilePath, settings );

    if ( isSuccessful )
    {
        closeProject();

        bool isOkToContinue = true;

        // Try to load the audio files
        foreach ( QString fileName, settings.audioFileNames )
        {
            const QString audioFilePath = projTempDir.absoluteFilePath( fileName );
            SharedSampleBuffer sampleBuffer = m_fileHandler.getSampleData( audioFilePath );

            if ( ! sampleBuffer.isNull() )
            {
                m_sampleBufferList << sampleBuffer;
            }
            else
            {
                isOkToContinue = false;
                break;
            }
        }

        // Try to read the audio file header info
        {
            const QString audioFilePath = projTempDir.absoluteFilePath( settings.audioFileNames.first() );
            m_sampleHeader = m_fileHandler.getSampleHeader( audioFilePath );
        }

        if ( m_sampleHeader.isNull() )
        {
            isOkToContinue = false;
        }

        // If the audio files were read successfully
        if ( isOkToContinue )
        {
            // Deal with sample ranges - provides backward compatibility with older save file format
            if ( ! settings.sampleRangeList.isEmpty() )
            {
                const int numChans = m_sampleHeader->numChans;

                QList<SharedSampleBuffer> tempSampleBuffers;

                foreach ( SharedSampleRange range, settings.sampleRangeList )
                {
                    SharedSampleBuffer sampleBuffer( new SampleBuffer( numChans, range->numFrames ) );

                    for ( int chanNum = 0; chanNum < numChans; chanNum++ )
                    {
                        sampleBuffer->copyFrom( chanNum, 0, *m_sampleBufferList.first().data(), chanNum, range->startFrame, range->numFrames );
                    }

                    tempSampleBuffers << sampleBuffer;
                }

                m_sampleBufferList = tempSampleBuffers;
            }

            // Only one sample buffer - waveform has not been sliced
            if ( m_sampleBufferList.size() == 1 )
            {
                const SharedWaveformItem item = m_graphicsScene->createWaveform( m_sampleBufferList.first(),
                                                                                 m_sampleHeader );
                connectWaveformToMainWindow( item );

                enableUI();

                if ( ! settings.slicePointFrameNums.isEmpty() )
                {
                    QUndoCommand* parentCommand = new QUndoCommand();

                    foreach ( int frameNum, settings.slicePointFrameNums )
                    {
                        new AddSlicePointItemCommand( frameNum, true, m_graphicsScene, m_ui->pushButton_Slice, m_ui->comboBox_SnapValues, parentCommand );
                    }
                    m_undoStack.push( parentCommand );
                }
            }
            else // Multiple sample buffers - waveform has been sliced
            {
                const QList<SharedWaveformItem> waveformItems = m_graphicsScene->createWaveforms( m_sampleBufferList, m_sampleHeader );

                foreach ( SharedWaveformItem item, waveformItems )
                {
                    connectWaveformToMainWindow( item );
                }

                enableUI();
                m_ui->actionAdd_Slice_Point->setEnabled( false );
                m_ui->pushButton_Find->setEnabled( false );
                m_ui->pushButton_Slice->setEnabled( true );
                m_ui->pushButton_Slice->setChecked( true );

            }

            m_appliedBPM = settings.appliedBpm;

            m_optionsDialog->setStretcherOptions( settings.options );

            if ( m_samplerAudioSource != NULL && m_rubberbandAudioSource != NULL )
            {
                const int startMidiNote = m_samplerAudioSource->getLowestAssignedMidiNote();

                QList<int> orderPositions;

                for ( int i = 0; i < settings.midiNotes.size() && i < settings.noteTimeRatios.size(); i++ )
                {
                    m_rubberbandAudioSource->setNoteTimeRatio( settings.midiNotes.at( i ),
                                                               settings.noteTimeRatios.at( i ) );

                    orderPositions << settings.midiNotes.at( i ) - startMidiNote;
                }

                m_graphicsScene->stretchWaveforms( orderPositions, settings.noteTimeRatios );
            }

            if ( settings.isJackSyncChecked )
            {
                m_optionsDialog->enableJackSync();
            }

            m_ui->checkBox_TimeStretch->setChecked( settings.isTimeStretchChecked );
            m_ui->checkBox_PitchCorrection->setChecked( settings.isPitchCorrectionChecked );

            if ( settings.originalBpm > 0.0 )
            {
                m_ui->doubleSpinBox_OriginalBPM->setValue( settings.originalBpm );
            }
            if ( settings.newBpm > 0.0 )
            {
                m_ui->doubleSpinBox_NewBPM->setValue( settings.newBpm );
            }

            {
                const int i = m_ui->comboBox_TimeSigNumerator->findText( QString::number( settings.timeSigNumerator ) );
                m_ui->comboBox_TimeSigNumerator->setCurrentIndex( i );
            }
            {
                const int i = m_ui->comboBox_TimeSigDenominator->findText( QString::number( settings.timeSigDenominator ) );
                m_ui->comboBox_TimeSigDenominator->setCurrentIndex( i );
            }
            m_ui->spinBox_Length->setValue( settings.length );
            m_ui->comboBox_Units->setCurrentIndex( settings.units );

            SamplerAudioSource::EnvelopeSettings envelopes;

            envelopes.attackValues = settings.attackValues;
            envelopes.releaseValues = settings.releaseValues;
            envelopes.oneShotSettings = settings.oneShotSettings;

            m_samplerAudioSource->setEnvelopeSettings( envelopes );

            // Clean up temp dir
            File( projTempDir.absolutePath().toLocal8Bit().data() ).deleteRecursively();

            // Store file path for later use unless under NSM management
            if ( m_nsmThread == NULL )
            {
                m_currentProjectFilePath = filePath;
                addPathToRecentProjects( filePath );
            }

            m_ui->statusBar->showMessage( tr("Project: ") + projectName );

            m_isProjectOpen = true;

            QApplication::restoreOverrideCursor();
        }
        else // Error loading audio files
        {
            m_sampleBufferList.clear();
            m_sampleHeader.clear();

            QApplication::restoreOverrideCursor();

            MessageBoxes::showWarningDialog( m_fileHandler.getLastErrorTitle(), m_fileHandler.getLastErrorInfo() );
        }
    }
}



void MainWindow::exportAs( const QString tempDirPath,
                           const QString outputDirPath,
                           const QString samplesDirPath,
                           const QString fileName,
                           const int exportType,
                           const int sndFileFormat,
                           const int outputSampleRate,
                           const int numSamplesToExport )
{
    QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

    const QDir outputDir( outputDirPath );

    const bool isExportTypeAudioFiles = exportType & ExportDialog::EXPORT_AUDIO_FILES;
    const bool isExportTypeH2Drumkit  = exportType & ExportDialog::EXPORT_H2DRUMKIT;
    const bool isExportTypeSFZ        = exportType & ExportDialog::EXPORT_SFZ;
    const bool isExportTypeAkaiPgm    = exportType & ExportDialog::EXPORT_AKAI_PGM;
    const bool isExportTypeMidiFile   = exportType & ExportDialog::EXPORT_MIDI_FILE;

    if ( isExportTypeAudioFiles )
    {
        outputDir.mkdir( fileName );
    }

    QStringList audioFileNames;
    bool isSuccessful = true;

    // Export audio files
    if ( isExportTypeAudioFiles )
    {
        for ( int i = 0; i < numSamplesToExport; i++ )
        {
            QString audioFileName = fileName;

            if ( isExportTypeAkaiPgm && audioFileName.size() > 14 )
            {
                audioFileName.resize( 14 );
            }

            if ( m_exportDialog->getNumberingStyle() == ExportDialog::NUMBERING_PREFIX )
            {
                audioFileName.prepend( QString::number( i + 1 ).rightJustified( 2, '0' ) );
            }
            else // SUFFIX
            {
                audioFileName.append( QString::number( i + 1 ).rightJustified( 2, '0' ) );
            }

            const QString path = m_fileHandler.saveAudioFile( samplesDirPath,
                                                              audioFileName,
                                                              m_sampleBufferList.at( i ),
                                                              m_sampleHeader->sampleRate,
                                                              outputSampleRate,
                                                              sndFileFormat );

            if ( ! path.isEmpty() )
            {
                if ( isExportTypeAkaiPgm )
                {
                    audioFileNames << audioFileName;    // File base name, no extension
                }
                else
                {
                    audioFileNames << QFileInfo( path ).fileName(); // File name including extension
                }
            }
            else
            {
                isSuccessful = false;
                break;
            }
        }
    }

    // Export Hydrogen drumkit
    if ( isSuccessful && isExportTypeH2Drumkit )
    {
        SamplerAudioSource::EnvelopeSettings envelopes;

        m_samplerAudioSource->getEnvelopeSettings( envelopes );

        TextFileHandler::createH2DrumkitXmlFile( samplesDirPath, fileName, audioFileNames, envelopes );
#ifdef LINUX
        const QString cdCommand  = "cd '" + outputDirPath + "'";
        const QString tarCommand = "tar --create --gzip --file '" + fileName + ".h2drumkit' '" + fileName + "'";

        const QString command = cdCommand + " && " + tarCommand;

        system( command.toLocal8Bit().data() );
#endif
        File( samplesDirPath.toLocal8Bit().data() ).deleteRecursively();
    }
    // Export SFZ
    else if ( isSuccessful && isExportTypeSFZ )
    {
        SamplerAudioSource::EnvelopeSettings envelopes;

        m_samplerAudioSource->getEnvelopeSettings( envelopes );

        const QString sfzFilePath = outputDir.absoluteFilePath( fileName + ".sfz" );
        const QString samplesDirName = QFileInfo( samplesDirPath ).fileName();

        TextFileHandler::createSFZFile( sfzFilePath, samplesDirName, audioFileNames, m_sampleBufferList, m_sampleHeader->sampleRate, envelopes );
    }
    // Export Akai PGM
    else if ( isSuccessful && isExportTypeAkaiPgm )
    {
        SamplerAudioSource::EnvelopeSettings envelopes;

        m_samplerAudioSource->getEnvelopeSettings( envelopes );

        const int modelID = m_exportDialog->getAkaiModelID();

        switch ( modelID )
        {
        case AkaiModelID::MPC1000_ID:
            AkaiFileHandler::writePgmFileMPC1000( audioFileNames, fileName, samplesDirPath, tempDirPath, envelopes );
            break;
        case AkaiModelID::MPC500_ID:
            AkaiFileHandler::writePgmFileMPC500( audioFileNames, fileName, samplesDirPath, tempDirPath, envelopes );
            break;
        default:
            break;
        }
    }

    // Export MIDI file
    if ( isSuccessful && isExportTypeMidiFile )
    {
        const qreal bpm = m_ui->doubleSpinBox_OriginalBPM->value();
        const ConfirmBpmDialog::TimeSigNumerator numerator = (ConfirmBpmDialog::TimeSigNumerator) m_ui->comboBox_TimeSigNumerator->currentIndex();
        const ConfirmBpmDialog::TimeSigDenominator denominator = (ConfirmBpmDialog::TimeSigDenominator) m_ui->comboBox_TimeSigDenominator->currentIndex();

        QApplication::restoreOverrideCursor();

        ConfirmBpmDialog dialog( bpm, numerator, denominator );
        const int result = dialog.exec();

        if ( result == QDialog::Accepted )
        {
            QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

            const qreal bpm = dialog.getBpm();
            const int timeSigNumerator = dialog.getTimeSigNumerator();
            const int timeSigDenominator = dialog.getTimeSigDenominator();
            const MidiFileHandler::MidiFileType type = (MidiFileHandler::MidiFileType) m_exportDialog->getMidiFileType();

            if ( isExportTypeAkaiPgm )
            {
                MidiFileHandler::SaveMidiFile( fileName, samplesDirPath, m_sampleBufferList, numSamplesToExport, m_sampleHeader->sampleRate, bpm, timeSigNumerator, timeSigDenominator, type );
            }
            else
            {
                MidiFileHandler::SaveMidiFile( fileName, outputDirPath, m_sampleBufferList, numSamplesToExport, m_sampleHeader->sampleRate, bpm, timeSigNumerator, timeSigDenominator, type );
            }
        }
    }

    QApplication::restoreOverrideCursor();

    if ( ! isSuccessful )
    {
        MessageBoxes::showWarningDialog( m_fileHandler.getLastErrorTitle(), m_fileHandler.getLastErrorInfo() );
    }
}



void MainWindow::saveProjectDialog()
{
    // Save file dialog
    const QString caption = m_nsmThread == NULL ? tr("Save Project") : tr("Export Project");
    const QString filter = tr("Shuriken Project") + "(*" + FILE_EXTENSION + ")";
    QString selectedFilter;
    QString filePath = QFileDialog::getSaveFileName( this, caption, m_lastOpenedProjDir, filter, &selectedFilter );

    // If user didn't click "Cancel"
    if ( ! filePath.isEmpty() )
    {
        QFileInfo projectFile( filePath );
        bool isFileNameChanged = false;

        if ( projectFile.completeSuffix() != "shuriken" )
        {
            QDir dir = projectFile.absoluteDir();
            QString newFileName = projectFile.baseName().append( FILE_EXTENSION );

            filePath = dir.absoluteFilePath( newFileName );
            projectFile.setFile( filePath );

            isFileNameChanged = true;
        }

        bool isOkToSave = true;

        if ( projectFile.exists() )
        {
            int buttonClicked = QMessageBox::Ok;

            if ( isFileNameChanged )
            {
                buttonClicked = MessageBoxes::showQuestionDialog( tr( "Overwrite existing file?" ),
                                                                  tr( "The file " ) + filePath + tr( " already exists.\n\nDo you want to overwrite this file?" ),
                                                                  QMessageBox::Ok | QMessageBox::Cancel );
            }

            if ( buttonClicked == QMessageBox::Ok )
            {
                isOkToSave = QFile::remove( filePath );

                if ( ! isOkToSave )
                {
                    MessageBoxes::showWarningDialog( tr( "Could not save project!" ),
                                                     tr( "The file \"" ) + filePath + tr( "\" could not be overwritten" ) );
                }
            }
            else // QMessageBox::Cancel
            {
                isOkToSave = false;
            }
        }

        if ( isOkToSave )
        {
            QFileInfo parentDir( projectFile.absolutePath() );

            if ( parentDir.isWritable() )
            {
                if ( m_nsmThread != NULL )
                {
                    saveProject( filePath, true );
                }
                else
                {
                    saveProject( filePath );
                }

                m_lastOpenedProjDir = projectFile.absolutePath();

            }
            else
            {
                MessageBoxes::showWarningDialog( tr( "Could not save project!" ),
                                                 tr( "Permission to write file denied" ) );
            }
        }
    }
}



void MainWindow::openProjectDialog()
{
    // Open file dialog
    const QString caption = m_nsmThread == NULL ? tr("Open Project") : tr("Import Project");
    const QString filter = tr("Shuriken Project") + "(*" + FILE_EXTENSION + ")";
    const QString filePath = QFileDialog::getOpenFileName( this, caption, m_lastOpenedProjDir, filter );

    // If user didn't click "Cancel"
    if ( ! filePath.isEmpty() )
    {
        openProject( filePath );

        if ( m_nsmThread != NULL )
        {
            m_nsmThread->sendMessage( NsmListenerThread::MSG_IS_DIRTY );
            m_ui->actionSave_Project->setEnabled( true );
        }

        m_lastOpenedProjDir = QFileInfo( filePath ).absolutePath();
    }
}



void MainWindow::importAudioFileDialog()
{
    // Open file dialog
    const QString filePath = QFileDialog::getOpenFileName( this, tr("Import Audio File"), m_lastOpenedImportDir,
                                                           tr("All Files (*.*)") );

    // If user didn't click "Cancel"
    if ( ! filePath.isEmpty() )
    {
        QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

        const QFileInfo fileInfo( filePath );
        const QString fileName = fileInfo.fileName();

        m_lastOpenedImportDir = fileInfo.absolutePath();

        SharedSampleBuffer sampleBuffer = m_fileHandler.getSampleData( filePath );
        SharedSampleHeader sampleHeader = m_fileHandler.getSampleHeader( filePath );

        if ( sampleBuffer.isNull() || sampleHeader.isNull() )
        {
            QApplication::restoreOverrideCursor();
            MessageBoxes::showWarningDialog( m_fileHandler.getLastErrorTitle(), m_fileHandler.getLastErrorInfo() );
        }
        else
        {
            closeProject();

            m_sampleBufferList << sampleBuffer;
            m_sampleHeader = sampleHeader;

            const SharedWaveformItem item = m_graphicsScene->createWaveform( sampleBuffer, sampleHeader );
            connectWaveformToMainWindow( item );

            setUpSampler();

            enableUI();

            // Set status bar message
            {
                const QString channels = sampleHeader->numChans == 1 ? "Mono" : "Stereo";

                const QString bits = QString::number( sampleHeader->bitsPerSample ) + " bits";

                const QString rate = QString::number( sampleHeader->sampleRate ) + " Hz";

                QString message = fileName + ", " + channels + ", " + bits + ", " + rate + ", " + sampleHeader->format;
                m_ui->statusBar->showMessage( message );
            }

            if ( m_nsmThread != NULL )
            {
                m_nsmThread->sendMessage( NsmListenerThread::MSG_IS_DIRTY );
                m_ui->actionSave_Project->setEnabled( true );
            }

            m_isProjectOpen = true;

            QApplication::restoreOverrideCursor();
        }
    }
}



void MainWindow::exportAsDialog()
{
    if ( m_exportDialog == NULL || m_optionsDialog == NULL )
    {
        return;
    }

    QPoint pos = m_exportDialog->pos();

    if ( pos.x() < 0 )
        pos.setX( 0 );
    if ( pos.y() < 0 )
        pos.setY( 0 );

    m_exportDialog->move( pos );

    const int result = m_exportDialog->exec();

    if ( result != QDialog::Accepted )
    {
        return;
    }

    const QString tempDirPath = m_optionsDialog->getTempDirPath();

    const QString outputDirPath = m_exportDialog->getOutputDirPath();
    const QDir outputDir( outputDirPath );

    const QString fileName = m_exportDialog->getFileName();

    const QString samplesDirPath = outputDir.absoluteFilePath( fileName );

    const bool isOverwriteEnabled = m_exportDialog->isOverwriteEnabled();

    const int exportType = m_exportDialog->getExportType();

    const bool isExportTypeAudioFiles = exportType & ExportDialog::EXPORT_AUDIO_FILES;
    const bool isExportTypeH2Drumkit  = exportType & ExportDialog::EXPORT_H2DRUMKIT;
    const bool isExportTypeSFZ        = exportType & ExportDialog::EXPORT_SFZ;
    const bool isExportTypeAkaiPgm    = exportType & ExportDialog::EXPORT_AKAI_PGM;
    const bool isExportTypeMidiFile   = exportType & ExportDialog::EXPORT_MIDI_FILE;

    const int sndFileFormat = m_exportDialog->getSndFileFormat();

    int outputSampleRate = m_exportDialog->getSampleRate();

    if ( outputSampleRate == ExportDialog::SAMPLE_RATE_KEEP_SAME )
    {
        outputSampleRate = m_sampleHeader->sampleRate;
    }

    int numSamplesToExport = m_sampleBufferList.size();

    QStringList fullFileNamesList;

    if ( isExportTypeH2Drumkit )
    {
        fullFileNamesList << fileName + ".h2drumkit";
    }
    else if ( isExportTypeSFZ )
    {
        fullFileNamesList << fileName + ".sfz";
    }
    else if ( isExportTypeAkaiPgm )
    {
        fullFileNamesList << fileName + AkaiFileHandler::getFileExtension();
    }
    else if ( isExportTypeMidiFile )
    {
        fullFileNamesList << fileName + MidiFileHandler::getFileExtension();
    }


    // Carry out some basic checks before exporting


    if ( ! QFileInfo( outputDirPath ).isWritable() )
    {
        MessageBoxes::showWarningDialog( tr( "Cannot export project!" ),
                                         tr( "Permission to write file(s) denied" ) );
        return;
    }

    if ( isExportTypeAudioFiles && QFileInfo( samplesDirPath ).exists() )
    {
        if ( isOverwriteEnabled )
        {
            const int buttonClicked = MessageBoxes::showQuestionDialog( tr( "Delete contents of directory?" ),
                                                                        tr( "Do you want to delete the contents of \"" ) + samplesDirPath + tr( "\" before continuing?" )
                                                                        + tr( "\n\nThis operation cannot be undone"),
                                                                        QMessageBox::Ok | QMessageBox::Cancel );
            if ( buttonClicked == QMessageBox::Ok )
            {
                File( samplesDirPath.toLocal8Bit().data() ).deleteRecursively();
            }
            else
            {
                return;
            }
        }
        else
        {
            MessageBoxes::showWarningDialog( tr( "Cannot export project!" ),
                                             tr( "Tried to create \"" ) + samplesDirPath + tr( "\" but this directory already exists" ) );
            return;
        }
    }

    foreach ( QString fullFileName, fullFileNamesList )
    {
        if ( outputDir.exists( fullFileName ) )
        {
            if ( isOverwriteEnabled )
            {
                QFile::remove( outputDir.absoluteFilePath( fullFileName ) );
            }
            else
            {
                MessageBoxes::showWarningDialog( tr( "Cannot export project!" ),
                                                 "\"" + fullFileName + tr( "\" already exists" ) );
                return;
            }
        }
    }

    if ( isExportTypeAkaiPgm )
    {
        const int modelID = m_exportDialog->getAkaiModelID();

        const int numPads = AkaiFileHandler::getNumPads( modelID );

        if ( numSamplesToExport > numPads )
        {
            numSamplesToExport = numPads;

            MessageBoxes::showWarningDialog( tr( "Cannot export all samples!" ),
                                             tr( "Too many samples for this Akai model, only the first " ) +
                                             QString::number( numPads ) + tr( " will be exported" ) );
        }
    }


    // Checks complete, now do export
    exportAs( tempDirPath, outputDirPath, samplesDirPath, fileName, exportType, sndFileFormat, outputSampleRate, numSamplesToExport );
}



void MainWindow::addPathToRecentProjects( QString filePath )
{
    TextFileHandler::PathsConfig config;

    TextFileHandler::readPathsConfigFile( config );

    // If this path is already in the recent projects list then remove it
    config.recentProjectPaths.removeOne( filePath );

    config.recentProjectPaths.prepend( filePath );

    while ( config.recentProjectPaths.size() > RecentProjects::MAX )
    {
        config.recentProjectPaths.removeLast();
    }

    TextFileHandler::createPathsConfigFile( config );

    m_ui->menuRecent_Projects->setEnabled( true );

    for ( int i = 0; i < config.recentProjectPaths.size(); i++ )
    {
        QString text = QFileInfo( config.recentProjectPaths.at( i ) ).fileName();

        m_recentProjectsActions.at( i )->setText( text );
        m_recentProjectsActions.at( i )->setData( config.recentProjectPaths.at( i ) );
        m_recentProjectsActions.at( i )->setVisible( true );
    }
}

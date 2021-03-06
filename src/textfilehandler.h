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

#ifndef TEXTFILEHANDLER_H
#define TEXTFILEHANDLER_H

#include <QString>
#include <QStringList>
#include "samplebuffer.h"
#include <rubberband/RubberBandStretcher.h>
#include "sampleraudiosource.h"

using namespace RubberBand;


class TextFileHandler
{
public:
    struct ProjectSettings
    {
        ProjectSettings() :
            originalBpm( 0.0 ),
            newBpm( 0.0 ),
            appliedBpm( 0.0 ),
            isTimeStretchChecked( false ),
            isPitchCorrectionChecked( false ),
            isJackSyncChecked( false ),
            options( 0 )
        {
        }

        QString projectName;
        QList<int> slicePointFrameNums;
        QStringList audioFileNames;
        QList<SharedSampleRange> sampleRangeList;   // Deprecated, exists only to provide backward compatibility
        qreal originalBpm;
        qreal newBpm;
        qreal appliedBpm;
        bool isTimeStretchChecked;
        bool isPitchCorrectionChecked;
        bool isJackSyncChecked;
        RubberBandStretcher::Options options;
        int timeSigNumerator;
        int timeSigDenominator;
        int length;
        int units;
        QList<int> midiNotes;
        QList<qreal> noteTimeRatios;
        QList<qreal> attackValues;
        QList<qreal> releaseValues;
        QList<bool> oneShotSettings;
    };

    static bool createProjectXmlFile( QString filePath, const ProjectSettings& settings );

    static bool readProjectXmlFile( QString filePath, ProjectSettings& settings );



    struct PathsConfig
    {
        QString tempDirPath;
        QStringList recentProjectPaths;
    };

    static bool createPathsConfigFile( const PathsConfig& config );

    static bool readPathsConfigFile( PathsConfig& config );



    static bool createH2DrumkitXmlFile( QString dirPath, QString kitName, QStringList audioFileNames,
                                        const SamplerAudioSource::EnvelopeSettings& envelopes );

    static bool createSFZFile( QString sfzFilePath,
                               QString samplesDirName,
                               QStringList audioFileNames,
                               QList<SharedSampleBuffer> sampleBufferList,
                               qreal sampleRate,
                               const SamplerAudioSource::EnvelopeSettings& envelopes );
};

#endif // TEXTFILEHANDLER_H

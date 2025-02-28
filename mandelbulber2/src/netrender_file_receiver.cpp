/*
 * netrender_file_receiver.cpp
 *
 *  Created on: 17 sie 2019
 *      Author: krzysztof
 */

#include "netrender_file_receiver.hpp"

#include "initparameters.hpp"
#include "system.hpp"

cNetRenderFileReceiver::cNetRenderFileReceiver(QObject *parent) : QObject(parent) {}

cNetRenderFileReceiver::~cNetRenderFileReceiver() = default;

void cNetRenderFileReceiver::ReceiveHeader(int clientIndex, qint64 _size, QString _fileName)
{
	if (!fileInfos.contains(clientIndex)) fileInfos.insert(clientIndex, sFileInfo());

	QString receivedFileName = _fileName;

	QString dirName;

	if (receivedFileName.left(4) == "DIR[")
	{
		int posOfBracket = receivedFileName.indexOf("]");
		dirName = receivedFileName.mid(4, posOfBracket - 1 - 3);
		receivedFileName = receivedFileName.mid(posOfBracket + 1);
	}

	int firstDash = receivedFileName.indexOf('_');
	QString fileName = receivedFileName.mid(firstDash + 1);

	QString fullFilePath = systemData.GetNetrenderFolder() + QDir::separator() + dirName + fileName;

	QFile file(fullFilePath);
	if (file.open(QIODevice::WriteOnly))
	{
		sFileInfo fileInfo;

		fileInfo.fileSize = _size;
		fileInfo.receivingStarted = true;
		fileInfo.dirName = dirName;
		fileInfo.chunkIndex = 0;
		fileInfo.fileName = fileName;
		fileInfo.fullFilePathInCache = fullFilePath;

		fileInfos[clientIndex] = fileInfo;

		file.close();
	}
	else
	{
		qCritical() << "Can't open file for write to NetRender cache " << fullFilePath;
	}
}

void cNetRenderFileReceiver::ReceiveChunk(int clientIndex, int chunkIndex, QByteArray data)
{
	if (fileInfos.contains(clientIndex))
	{
		sFileInfo fileInfo = fileInfos[clientIndex];
		if (chunkIndex == fileInfo.chunkIndex + 1)
		{
			qint64 bytesLeft =
				fileInfo.fileSize - fileInfo.chunkIndex * cNetRenderFileReceiver::CHUNK_SIZE;
			qint64 expectedChunkSize = qMin(bytesLeft, cNetRenderFileReceiver::CHUNK_SIZE);
			if (data.size() == expectedChunkSize)
			{
				QFile file(fileInfo.fullFilePathInCache);

				if (file.open(QIODevice::Append))
				{
					file.write(data);
					file.close();
					fileInfo.chunkIndex++;
				}
				else
				{
					qCritical() << "Can't open file for write to NetRender cache "
											<< fileInfo.fullFilePathInCache;
				}

				// last chunk
				if (bytesLeft == expectedChunkSize)
				{
					fileInfo.receivingStarted = false;

					QString destFileName;

					if (fileInfo.dirName.isEmpty())
					{
						destFileName = gPar->Get<QString>("anim_keyframe_dir") + fileInfo.fileName;
					}
					else
					{
						QDir dir(gPar->Get<QString>("anim_keyframe_dir"));
						if (dir.exists())
						{
							if (!dir.exists(fileInfo.dirName))
							{
								dir.mkdir(fileInfo.dirName);
							}
						}
						destFileName = gPar->Get<QString>("anim_keyframe_dir") + fileInfo.dirName
													 + QDir::separator() + fileInfo.fileName;
					}

					file.copy(destFileName);
					file.remove();
				}
			}
			else
			{
				qCritical() << "ReceiveChunk(): Wrong chunk size" << data.size() << expectedChunkSize;
			}
		}
		else
		{
			qCritical() << "ReceiveChunk(): Wrong chunk index" << chunkIndex;
		}

		fileInfos[clientIndex] = fileInfo;
	}
	else
	{
		qCritical() << "ReceiveChunk(): Unknown client index" << clientIndex;
	}
}

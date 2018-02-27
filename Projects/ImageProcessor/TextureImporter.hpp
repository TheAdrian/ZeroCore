// Authors: Nathan Carlson
// Copyright 2015, DigiPen Institute of Technology

#pragma once

namespace Zero
{

void ResizeImage(TextureFormat::Enum format, const byte* srcImage, uint srcWidth, uint srcHeight, byte* dstImage, uint dstWidth, uint dstHeight);

void ToNvttSurface(nvtt::Surface& surface, uint width, uint height, TextureFormat::Enum format, const byte* image);
void FromNvttSurface(const nvtt::Surface& surface, uint& width, uint& height, TextureFormat::Enum format, byte*& image);

class TextureImporter
{
public:
  TextureImporter(StringParam inputFile, StringParam outputFile, StringParam metaFile);
  ~TextureImporter();

  void ProcessTexture(Status& status);

  void LoadImageData(Status& status, StringParam extension);

  void WriteTextureFile(Status& status);

  String mInputFile;
  String mOutputFile;
  String mMetaFile;

  ImageContent* mImageContent;
  TextureBuilder* mBuilder;

  TextureFormat::Enum mLoadFormat;
  Array<MipHeader> mMipHeaders;
  Array<byte*> mImageData;

  Array<MipHeader> mBackupMipHeaders;
  Array<byte*> mBackupImageData;

  bool mMetaChanged;
};

} // namespace Zero

@echo off
rem �\����X�V�̃^�C�~���O�ł���ڂ��J�����_�[�ɗ\����A�b�v���[�h����T���v���o�b�`
rem _EDCBX_DIRECT_
rem _EDCBX_#HIDE_ (#����菜���ƃE�B���h�E��\���ɂȂ�)

rem v3.5�ȏ��C#�R���p�C���̃p�X���w�肷��
set CSC_PATH=%SystemRoot%\Microsoft.NET\Framework\v4.0.30319\csc.exe

rem ���ڎ��s���\����X�V�̂Ƃ�����
if not defined NotifyID set NotifyID=2
if "%NotifyID%"=="2" (
  if not exist EdcbSchUploader.exe (
    if not exist EdcbSchUploader.cs (
      echo �G���[: EdcbSchUploader.cs ��������܂���B
      goto EOF
    )
    if not exist %CSC_PATH% (
      echo �G���[: %CSC_PATH% ��������܂���B
      goto EOF
    )
    echo EdcbSchUploader.cs ���R���p�C�����܂��B
    %CSC_PATH% /nologo /optimize EdcbSchUploader.cs
    if not exist EdcbSchUploader.exe (
      echo �G���[: EdcbSchUploader.exe ��������܂���B
      goto EOF
    )
  )

  echo ����ڂ��J�����_�[�ɗ\����A�b�v���[�h���܂��B
  EdcbSchUploader.exe �y���[�UID�ƃp�X���[�h�������Ɏw��z

:EOF
  rem ���퓊�����͂���pause����菜��
  pause
)

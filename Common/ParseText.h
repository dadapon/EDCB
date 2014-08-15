#pragma once

#include "StringUtil.h"

template <class K, class V>
class CParseText
{
public:
	CParseText() : saveWide(false) {}
	virtual ~CParseText() {}
	bool ParseText(LPCWSTR filePath = NULL);
	const map<K, V>& GetMap() const { return this->itemMap; }
	const wstring& GetFilePath() const { return this->filePath; }
	void SetFilePath(LPCWSTR filePath) { this->filePath = filePath; this->saveWide = false; }
protected:
	bool SaveText() const;
	virtual bool ParseLine(const wstring& parseLine, pair<K, V>& item) = 0;
	virtual bool SaveLine(const pair<K, V>& item, wstring& saveLine) const { return false; }
	virtual bool SaveFooterLine(wstring& saveLine) const { return false; }
	virtual bool SelectIDToSave(vector<K>& sortList) const { return false; }
	map<K, V> itemMap;
	wstring filePath;
	bool saveWide;
};

template <class K, class V>
bool CParseText<K, V>::ParseText(LPCWSTR filePath)
{
	this->itemMap.clear();
	this->saveWide = false;
	if( filePath != NULL ){
		this->filePath = filePath;
	}
	if( this->filePath.empty() ){
		return false;
	}
	HANDLE hFile;
	for( int retry = 0;; ){
		hFile = CreateFile(this->filePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if( hFile != INVALID_HANDLE_VALUE ){
			break;
		}else if( GetLastError() == ERROR_FILE_NOT_FOUND ){
			return true;
		}else if( ++retry > 5 ){
			//6回トライしてそれでもダメなら失敗
			OutputDebugString(L"CParseText<>::ParseText(): Error: Cannot open file\r\n");
			return false;
		}
		Sleep(200 * retry);
	}

	bool checkWide = false;
	vector<char> buf;
	string parseLineA;
	wstring parseLine;
	for(;;){
		//4KB単位で読み込む
		buf.resize(buf.size() + 4096);
		DWORD dwRead;
		if( ReadFile(hFile, &buf.front() + buf.size() - 4096, 4096, &dwRead, NULL) == FALSE || dwRead == 0 ){
			buf.resize(buf.size() - 4096);
			buf.insert(buf.end(), 3, '\0');
		}else{
			buf.resize(buf.size() - 4096 + dwRead);
		}
		if( checkWide == false ){
			checkWide = true;
			if( buf.size() >= 2 && buf[0] == '\xFF' && buf[1] == '\xFE' ){
				this->saveWide = true;
				buf.erase(buf.begin(), buf.begin() + 2);
			}
		}
		//完全に読み込まれた行をできるだけ解析
		if( this->saveWide ){
			for( size_t i = 0; i + 1 < buf.size(); i += 2 ){
				if( buf[i] == '\0' && buf[i + 1] == '\0' ||
				    buf[i] == '\r' && buf[i + 1] == '\0' && i + 3 < buf.size() && buf[i + 2] == '\n' && buf[i + 3] == '\0' ){
					parseLine.clear();
					for( size_t j = 0; j + 1 < i; j += 2 ){
						parseLine.push_back((buf[j] & 0xFF) | buf[j + 1] << 8);
					}
					pair<K, V> item;
					if( ParseLine(parseLine, item) ){
						this->itemMap.insert(item);
					}
					if( buf[i] == '\0' ){
						buf.erase(buf.begin(), buf.begin() + i);
						break;
					}
					buf.erase(buf.begin(), buf.begin() + i + 4);
					i = 0;
				}
			}
			if( buf.size() >= 2 && buf[0] == '\0' && buf[1] == '\0' ){
				break;
			}
		}else{
			for( size_t i = 0; i < buf.size(); i++ ){
				if( buf[i] == '\0' || buf[i] == '\r' && i + 1 < buf.size() && buf[i + 1] == '\n' ){
					parseLineA.assign(buf.begin(), buf.begin() + i);
					AtoW(parseLineA, parseLine);
					pair<K, V> item;
					if( ParseLine(parseLine, item) ){
						this->itemMap.insert(item);
					}
					if( buf[i] == '\0' ){
						buf.erase(buf.begin(), buf.begin() + i);
						break;
					}
					buf.erase(buf.begin(), buf.begin() + i + 2);
					i = 0;
				}
			}
			if( buf.empty() == false && buf[0] == '\0' ){
				break;
			}
		}
	}
	CloseHandle(hFile);
	return true;
}

template <class K, class V>
bool CParseText<K, V>::SaveText() const
{
	if( this->filePath.empty() ){
		return false;
	}
	HANDLE hFile;
	for( int retry = 0;; ){
		hFile = _CreateDirectoryAndFile(this->filePath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if( hFile != INVALID_HANDLE_VALUE ){
			break;
		}else if( ++retry > 5 ){
			OutputDebugString(L"CParseText<>::SaveText(): Error: Cannot open file\r\n");
			return false;
		}
		Sleep(200 * retry);
	}

	DWORD dwWrite;
	if( this->saveWide ){
		WriteFile(hFile, L"\uFEFF", 2, &dwWrite, NULL);
	}
	wstring saveLine;
	string saveLineA;
	vector<K> idList;
	if( SelectIDToSave(idList) ){
		for( size_t i = 0; i < idList.size(); i++ ){
			map<K, V>::const_iterator itr = this->itemMap.find(idList[i]);
			saveLine.clear();
			if( itr != this->itemMap.end() && SaveLine(*itr, saveLine) ){
				if( this->saveWide ){
					saveLine += L"\r\n";
					WriteFile(hFile, saveLine.c_str(), (DWORD)saveLine.size() * 2, &dwWrite, NULL);
				}else{
					WtoA(saveLine, saveLineA);
					saveLineA += "\r\n";
					WriteFile(hFile, saveLineA.c_str(), (DWORD)saveLineA.size(), &dwWrite, NULL);
				}
			}
		}
	}else{
		for( map<K, V>::const_iterator itr = this->itemMap.begin(); itr != this->itemMap.end(); itr++ ){
			saveLine.clear();
			if( SaveLine(*itr, saveLine) ){
				if( this->saveWide ){
					saveLine += L"\r\n";
					WriteFile(hFile, saveLine.c_str(), (DWORD)saveLine.size() * 2, &dwWrite, NULL);
				}else{
					WtoA(saveLine, saveLineA);
					saveLineA += "\r\n";
					WriteFile(hFile, saveLineA.c_str(), (DWORD)saveLineA.size(), &dwWrite, NULL);
				}
			}
		}
	}
	saveLine.clear();
	if( SaveFooterLine(saveLine) ){
		if( this->saveWide ){
			saveLine += L"\r\n";
			WriteFile(hFile, saveLine.c_str(), (DWORD)saveLine.size() * 2, &dwWrite, NULL);
		}else{
			WtoA(saveLine, saveLineA);
			saveLineA += "\r\n";
			WriteFile(hFile, saveLineA.c_str(), (DWORD)saveLineA.size(), &dwWrite, NULL);
		}
	}
	CloseHandle(hFile);
	return true;
}

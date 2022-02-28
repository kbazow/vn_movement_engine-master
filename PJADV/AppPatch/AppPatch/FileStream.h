// FileStream.h: interface for the CFileStream class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FILESTREAM_H__F652F356_495C_4C6D_A3B9_C59BCE577810__INCLUDED_)
#define AFX_FILESTREAM_H__F652F356_495C_4C6D_A3B9_C59BCE577810__INCLUDED_

#include <vector>
using namespace std;
typedef struct file_header_s
{
	char Sign[32];      //ǰ�ñ��
	DWORD strCount;
	DWORD strData;
	DWORD treeList;
	DWORD treeDataSize;
	DWORD treeCompSize;
	DWORD treeData;
}file_header_s;
typedef struct stringtable_t
{
	DWORD size;			//�ַ����Ĵ�С
	char* data;			//�ַ��������ݵ�ַ
}stringtable_s;
typedef struct memory_tree_t
{
	BOOL bDirectory;    //�Ƿ���Ŀ¼
	DWORD dwHashValue;  //�ļ����Ĺ�ϣֵ,���ڿ���Ѱַ
	DWORD nStringIdx;   //�ַ�������ֵ,���ڶ�λStringλ��(�ļ���)
	DWORD dataSize;     //�����ļ���С
	vector <memory_tree_t*> dataList;
	PBYTE dataOffset;   //ָ���ļ�����
}memory_tree_s;
typedef struct disk_tree_s
{
	BOOL bDirectory;    //�Ƿ���Ŀ¼
	DWORD dwHashValue;  //�ļ����Ĺ�ϣֵ,���ڿ���Ѱַ
	DWORD fileCount;	//�ļ�����
	DWORD nStringIdx;   //�ַ�������ֵ,���ڶ�λStringλ��(�ļ���)
	DWORD dataSize;     //�����ļ���С
	DWORD dataOffset;   //ָ���ļ�����
}disk_tree_t;
typedef struct save_data_s
{
	DWORD strSize;
	DWORD strCount;
	DWORD treeSize;
	DWORD treeDataSize;
	DWORD treeCompSize;
	char* strData;
	disk_tree_t* treeList;
	void* treeData;
}save_data_t;
enum FileStreamType
{
	FileOpen,
	FileCreate
};

#define SIGN_STRING "����"
class CFileStream  
{
private:
	
	file_header_s fileHeader;

	memory_tree_s memoryTreeRoot;
	vector <stringtable_s*> memoryString;

	PBYTE memoryData;
	HANDLE fileHandle;
	FileStreamType fileType;

	DWORD fileDataSize;

	bool m_strcmp(int idx,char* string);
	bool RecursionInsertFile(char* treepath,memory_tree_s* fileTree,memory_tree_s* DirTree=NULL);
	memory_tree_s* RecursionFileExists(char* TreePath,memory_tree_s* DirTree);
	
	void RecursionTreeList(memory_tree_s* root,save_data_t* save);
	void RecursionTreeList(disk_tree_s* tree,void* treedata,PDWORD offset,DWORD startaddr=0,memory_tree_s* List=NULL);

	BOOL RecursionDirectory(char* lpszDir ,int start=0);
public:
	CFileStream();
	virtual ~CFileStream();
	bool Open(char* pszFilePath);
	bool Create(char* pszFilePath);
	DWORD GetHashValue(char* str);
	memory_tree_s* FileIsExists(char* TreePath);
	char* GetFolderName(char* TreePath);
	char* GetFileName(char* TreePath);
	memory_tree_s * GetFile(char* treepath);
	char* GetName(int idx);
	int AllocString(char* str);
	bool SaveToStream(BOOL bCompress=TRUE);
	bool LoadFromStream();
	bool AddFile(char* pszFilePath,char* treepath);
};

#endif // !defined(AFX_FILESTREAM_H__F652F356_495C_4C6D_A3B9_C59BCE577810__INCLUDED_)

#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include "filehandler.h"
#include "serializers.h" // remove this
using namespace std;


bool isValidPath(const char* path){
  struct stat info;

  if(stat(path, &info) != 0){
    return false;
  }
  return true;
}

bool isValidPath(const string& path){
  return isValidPath(path.c_str());
}

unsigned int getFileSize(const string& path){
  if(!isValidPath(path)){
    throw std::runtime_error("Given path is invalid.");
  }

  ifstream in(path, std::ifstream::ate | std::ifstream::binary);
  return in.tellg();
}

int itemMatchCredential(const string& storedFilePath, Credential& credential){
  /*
  file_size = os.path.getsize(file_path)

  with open(file_path, 'rb+') as f:
      read_ptr = item_size

      while abs(read_ptr) <= abs(file_size):
          f.seek(-read_ptr, os.SEEK_END)
          item = f.read(item_size).decode('utf-8')

          if compare_func(item, **compare_kwargs):
              return read_ptr

          read_ptr += item_size

  return None
  */
  unsigned int fileSize = getFileSize(storedFilePath);
  unsigned int itemSize = configParams["SERIAL_SIZE_CREDENTIAL"];
  int readPtr = itemSize;

  FILE* matchedFile;
  matchedFile = fopen(storedFilePath.c_str(), "r+");

  char item[itemSize];

  while(readPtr <= fileSize){
    fseek(matchedFile, -readPtr, SEEK_END);
    fread(item, itemSize, 1, matchedFile);
    item[itemSize] = '\0'; // Cap the garbage (without this, garbage is appended)

    if(matchesCredential(string(item), credential)){
      fclose(matchedFile);
      return readPtr;
    }

    readPtr += itemSize;
  }

  fclose(matchedFile);
  return -1;
}


int main(){
  configServer();

  string username = "petaarov";
  string filePath = getStoredFilePath(StoredFileType::CredentialFile, username);

  cout << filePath << "\n";

  //cout << getFileSize(filePath) << "\n";

  Credential cred = {
    Active::No,
    "5555user",
    "5555password"
  };
  string ser = serializeCredential(cred);
  //cout << ser << "\n";

  cout << itemMatchCredential(filePath, cred) << "\n";
}

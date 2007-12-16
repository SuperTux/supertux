/* PhysFS_DLL - (c)2003 Gregory S. Read
 * Internal class that provides direct access to the PhysFS DLL.  It is
 * not accessible outside of the PhysFS.NET assembly.
 */
using System.Collections;
using System.Runtime.InteropServices;

namespace PhysFS_NET
{
   internal class PhysFS_DLL
   {
      /* Static constructor
       * Initializes the PhysFS API before any method is called in this class.  This
       * relieves the user from having to explicitly initialize the API.
       * Parameters
       *    none
       * Returns
       *    none
       * Exceptions
       *    PhysFSException - An error occured in the PhysFS API
       */
      static PhysFS_DLL()
      {
         if(PHYSFS_init("") == 0)
            throw new PhysFSException();
      }

      /* BytePPToArray
       * Converts a C-style string array into a .NET managed string array
       * Parameters
       *    C-style string array pointer returned from PhysFS
       * Returns
       *    .NET managed string array
       * Exceptions
       *    none
       */
      public unsafe static string[] BytePPToArray(byte **bytearray)
      {
         byte** ptr;
         byte* c;
         string tempstr;
         ArrayList MyArrayList = new ArrayList();
         string[] RetArray;

         for(ptr = bytearray; *ptr != null; ptr++)
         {
            tempstr = "";
            for(c = *ptr; *c != 0; c++)
            {
               tempstr += (char)*c;
            }

            // Add string to our list
            MyArrayList.Add(tempstr);
         }

         // Return a normal array of the list
         RetArray = new string[MyArrayList.Count];
         MyArrayList.CopyTo(RetArray, 0);
         return RetArray;
      }

      // Name of DLL to import
      private const string PHYSFS_DLLNAME = "physfs.dll";

      // DLL import declarations
      [DllImport(PHYSFS_DLLNAME)] public static extern int PHYSFS_init(string argv0);
      [DllImport(PHYSFS_DLLNAME)] public static extern int PHYSFS_deinit();
      [DllImport(PHYSFS_DLLNAME)] public static extern unsafe void PHYSFS_freeList(void *listVar);
      [DllImport(PHYSFS_DLLNAME)] public static extern string PHYSFS_getLastError();
      [DllImport(PHYSFS_DLLNAME)] public static extern string PHYSFS_getDirSeparator();
      [DllImport(PHYSFS_DLLNAME)] public static extern void PHYSFS_permitSymbolicLinks(int allow);
      [DllImport(PHYSFS_DLLNAME)] public static extern unsafe byte** PHYSFS_getCdRomDirs();
      [DllImport(PHYSFS_DLLNAME)] public static extern string PHYSFS_getBaseDir();
      [DllImport(PHYSFS_DLLNAME)] public static extern string PHYSFS_getUserDir();
      [DllImport(PHYSFS_DLLNAME)] public static extern string PHYSFS_getWriteDir();
      [DllImport(PHYSFS_DLLNAME)] public static extern int PHYSFS_setWriteDir(string newDir);
      [DllImport(PHYSFS_DLLNAME)] public static extern int PHYSFS_addToSearchPath(string newDir, int appendToPath);
      [DllImport(PHYSFS_DLLNAME)] public static extern int PHYSFS_removeFromSearchPath(string oldDir);
      [DllImport(PHYSFS_DLLNAME)] public static extern unsafe byte** PHYSFS_getSearchPath();
      [DllImport(PHYSFS_DLLNAME)] public static extern int PHYSFS_setSaneConfig(string organization,
         string appName,
         string archiveExt,
         int includeCdRoms,
         int archivesFirst);
      [DllImport(PHYSFS_DLLNAME)] public static extern int PHYSFS_mkdir(string dirName);
      [DllImport(PHYSFS_DLLNAME)] public static extern int PHYSFS_delete(string filename);
      [DllImport(PHYSFS_DLLNAME)] public static extern string PHYSFS_getRealDir(string filename);
      [DllImport(PHYSFS_DLLNAME)] public static extern unsafe byte** PHYSFS_enumerateFiles(string dir);
      [DllImport(PHYSFS_DLLNAME)] public static extern int PHYSFS_exists(string fname);
      [DllImport(PHYSFS_DLLNAME)] public static extern int PHYSFS_isDirectory(string fname);
      [DllImport(PHYSFS_DLLNAME)] public static extern int PHYSFS_isSymbolicLink(string fname);
      [DllImport(PHYSFS_DLLNAME)] public static extern unsafe void* PHYSFS_openWrite(string filename);
      [DllImport(PHYSFS_DLLNAME)] public static extern unsafe void* PHYSFS_openAppend(string filename);
      [DllImport(PHYSFS_DLLNAME)] public static extern unsafe void* PHYSFS_openRead(string filename);
      [DllImport(PHYSFS_DLLNAME)] public static extern unsafe int PHYSFS_close(void* handle);
      [DllImport(PHYSFS_DLLNAME)] public static extern unsafe long PHYSFS_getLastModTime(string filename);
      [DllImport(PHYSFS_DLLNAME)] public static extern unsafe long PHYSFS_read(void* handle,
         void *buffer,
         uint objSize,
         uint objCount);
      [DllImport(PHYSFS_DLLNAME)] public static extern unsafe long PHYSFS_write(void* handle,
         void *buffer,
         uint objSize,
         uint objCount);
      [DllImport(PHYSFS_DLLNAME)] public static extern unsafe int PHYSFS_eof(void* handle);
      [DllImport(PHYSFS_DLLNAME)] public static extern unsafe long PHYSFS_tell(void* handle);
      [DllImport(PHYSFS_DLLNAME)] public static extern unsafe int PHYSFS_seek(void* handle, ulong pos);
      [DllImport(PHYSFS_DLLNAME)] public static extern unsafe long PHYSFS_fileLength(void* handle);
      [DllImport(PHYSFS_DLLNAME)] public static extern unsafe int PHYSFS_setBuffer(void* handle, ulong bufsize);
      [DllImport(PHYSFS_DLLNAME)] public static extern unsafe int PHYSFS_flush(void* handle);
   }
}

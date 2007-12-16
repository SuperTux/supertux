/* PhysFS.cs - (c)2003 Gregory S. Read
 * Provides access to PhysFS API calls not specific to file handle access.
 */
using System;

namespace PhysFS_NET
{
   public class PhysFS
   {
      /* Initialize
       * Inits the PhysFS API.  This normally does not need to be called unless
       * the API has been manually deinitialized since the PhysFS_DLL class
       * initializes just before the first call is made into the DLL.
       * Parameters
       *    none
       * Returns
       *    none
       * Exceptions
       *    PhysFSException - An error occured in the PhysFS API
       */
      public static void Initialize()
      {
         // Initialize the physfs library, raise an exception if error
         if(PhysFS_DLL.PHYSFS_init("") == 0)
            throw new PhysFSException();
      }

      /* Deinitialize
       * Deinits the PhysFS API.  It is recommended that this method be called
       * by the application before exiting in order to gracefully deallocate
       * resources and close all filehandles, etc.
       * Parameters
       *    none
       * Returns
       *    none
       * Exceptions
       *    PhysFSException - An error occured in the PhysFS API
       */
      public static void Deinitialize()
      {
         // Deinit, raise an exception if an error occured
         if(PhysFS_DLL.PHYSFS_deinit() == 0)
            throw new PhysFSException();
      }

      /* BaseDir
       * Gets the base directory configured for PhysFS.  See the PhysFS API
       * documentation for more information.
       * Parameters
       *    none
       * Returns
       *    A string value representing the Base Directory
       * Exceptions
       *    none
       */
      public static string BaseDir
      {
         get
         {
            // Return the current base directory
            return PhysFS_DLL.PHYSFS_getBaseDir();
         }
      }

      /* WriteDir
       * Gets or sets the write directory configured for PhysFS.  See the PhysFS API
       * documentation for more information.
       * Parameters
       *    set - Path to set the WriteDir property to
       * Returns
       *    A string value representing the Write Directory
       * Exceptions
       *    PhysFSException - An error occured in the PhysFS API when
       *       settings the write directory.
       */
      public static string WriteDir
      {
         get
         {
            // Return the current write directory
            return PhysFS_DLL.PHYSFS_getWriteDir();
         }
         set
         {
            // Set the write directory and raise an exception if an error occured
            if(PhysFS_DLL.PHYSFS_setWriteDir(value) == 0)
               throw new PhysFSException();
         }
      }

      /* UserDir
       * Gets or sets the write directory configured for PhysFS.  See the PhysFS API
       * documentation for more information.
       * Parameters
       *    set - Path to set the WriteDir property to
       * Returns
       *    A string value representing the Write Directory
       * Exceptions
       *    PhysFSException - An error occured in the PhysFS API when
       *       settings the write directory.
       */
      public static string UserDir
      {
         get
         {
            // Return the current user directory
            return PhysFS_DLL.PHYSFS_getUserDir();
         }
      }
      public static void AddToSearchPath(string NewDir, bool Append)
      {
         if(PhysFS_DLL.PHYSFS_addToSearchPath(NewDir, Append?1:0) == 0)
            throw new PhysFSException();
      }
      public static void RemoveFromSearchPath(string OldDir)
      {
         if(PhysFS_DLL.PHYSFS_removeFromSearchPath(OldDir) == 0)
            throw new PhysFSException();
      }
      public unsafe static string[] GetSearchPath()
      {
         byte** p;				// Searchpath list from PhysFS dll
         string[] pathlist;	// List converted to an array

         // Get the CDROM drive listing
         p = PhysFS_DLL.PHYSFS_getSearchPath();
         // Convert the C-style array to a .NET style array
         pathlist = PhysFS_DLL.BytePPToArray(p);
         // Free the original list since we're done with it
         PhysFS_DLL.PHYSFS_freeList(p);

         return pathlist;
      }
      public unsafe static string[] GetCDROMDrives()
      {
         byte** p;				// CDROM list from PhysFS dll
         string[] cdromlist;	// List converted to an array

         // Get the CDROM drive listing
         p = PhysFS_DLL.PHYSFS_getCdRomDirs();
         // Convert the C-style array to a .NET style array
         cdromlist = PhysFS_DLL.BytePPToArray(p);
         // Free the original list since we're done with it
         PhysFS_DLL.PHYSFS_freeList(p);

         return cdromlist;
      }
      public static void MkDir(string Dirname)
      {
         if(PhysFS_DLL.PHYSFS_mkdir(Dirname) == 0)
            throw new PhysFSException();
      }
      public static void Delete(string Filename)
      {
         if(PhysFS_DLL.PHYSFS_delete(Filename) == 0)
            throw new PhysFSException();
      }
      public static string GetRealDir(string Filename)
      {
         string RetValue;

         RetValue = PhysFS_DLL.PHYSFS_getRealDir(Filename);
         if(RetValue == null)
            throw new PhysFSException("File not found in search path.");

         // Return the real file path of the specified filename
         return RetValue;
      }
      public unsafe static string[] EnumerateFiles(string Dirname)
      {
         byte** p;				// File list from PhysFS dll
         string[] filelist;	// List converted to an array

         // Get the CDROM drive listing
         p = PhysFS_DLL.PHYSFS_enumerateFiles(Dirname);
         // Convert the C-style array to a .NET style array
         filelist = PhysFS_DLL.BytePPToArray(p);
         // Free the original list since we're done with it
         PhysFS_DLL.PHYSFS_freeList(p);

         return filelist;
      }
      public static bool IsDirectory(string Filename)
      {
         // Return true if non-zero, otherwise return false
         return (PhysFS_DLL.PHYSFS_isDirectory(Filename) == 0)?false:true;
      }
   }
}

/* PhysFSFileStream.cs - (c)2003 Gregory S. Read */
using System;
using System.Collections;
using System.IO;

namespace PhysFS_NET
{
   public enum PhysFSFileMode {Read, Write, Append};

   // Our exception class we'll use for throwing all PhysFS API related exception
   public class PhysFSException : IOException
   {
      public PhysFSException(string Message) : base(Message) {}
      public PhysFSException() : base(PhysFS_DLL.PHYSFS_getLastError()) {}
   }

   public unsafe class PhysFSFileStream : Stream
   {
      // ***Public properties***
      public override bool CanRead
      {
         get
         {
            // Reading is supported
            return true;
         }
      }
      
      public override bool CanSeek
      {
         get
         {
            // Seek is supported
            return true;
         }
      }

      public override bool CanWrite
      {
         get
         {
            // Writing is supported
            return true;
         }
      }

      public override long Length
      {
         get
         {
            long TempLength;
            TempLength = PhysFS_DLL.PHYSFS_fileLength(pHandle);

            // If call returned an error, throw an exception
            if(TempLength == -1)
               throw new PhysFSException();

            return TempLength;
         }
      }

      public override long Position
      {
         get
         {
            long TempPosition;
            TempPosition = PhysFS_DLL.PHYSFS_tell(pHandle);

            // If call returned an error, throw an exception
            if(TempPosition == -1)
               throw new PhysFSException();

            return TempPosition;
         }
         set
         {
            // Seek from beginning of file using the position value
            Seek(value, SeekOrigin.Begin);
         }
      }
      
      // ***Public methods***
      public PhysFSFileStream(string FileName, PhysFSFileMode FileMode, ulong BufferSize)
      {
         // Open the specified file with the appropriate file access
         switch(FileMode)
         {
            case PhysFSFileMode.Read:
               pHandle = PhysFS_DLL.PHYSFS_openRead(FileName);
               break;
            case PhysFSFileMode.Write:
               pHandle = PhysFS_DLL.PHYSFS_openWrite(FileName);
               break;
            case PhysFSFileMode.Append:
               pHandle = PhysFS_DLL.PHYSFS_openAppend(FileName);
               break;
            default:
               throw new PhysFSException("Invalid FileMode specified");
         }

         // If handle is null, an error occured, so raise an exception
         //!!! Does object get created if exception is thrown?
         if(pHandle == null)
            throw new PhysFSException();

         // Set buffer size, raise an exception if an error occured
         if(PhysFS_DLL.PHYSFS_setBuffer(pHandle, BufferSize) == 0)
            throw new PhysFSException();
      }

      // This constructor sets the buffer size to 0 if not specified
      public PhysFSFileStream(string FileName, PhysFSFileMode FileMode) : this(FileName, FileMode, 0) {}
		
      ~PhysFSFileStream()
      {
         // Don't close the handle if they've specifically closed it already
         if(!Closed)
            Close();
      }

      public override void Flush()
      {
         if(PhysFS_DLL.PHYSFS_flush(pHandle) == 0)
            throw new PhysFSException();
      }

      public override int Read(byte[] buffer, int offset, int count)
      {
         long RetValue;
   
         fixed(byte *pbytes = &buffer[offset])
         {
            // Read into our allocated pointer
            RetValue = PhysFS_DLL.PHYSFS_read(pHandle, pbytes, sizeof(byte), (uint)count);
         }

         if(RetValue == -1)
            throw new PhysFSException();

         // Return number of bytes read
         // Note: This cast should be safe since we are only reading 'count' items, which
         // is of type 'int'.
         return (int)RetValue;
      }

      public override void Write(byte[] buffer, int offset, int count)
      {
         long RetValue;

         fixed(byte* pbytes = &buffer[offset])
         {
            // Write buffer
            RetValue = PhysFS_DLL.PHYSFS_write(pHandle, pbytes, sizeof(byte), (uint)count);
         }

         if(RetValue == -1)
            throw new PhysFSException();
      }

      public override long Seek(long offset, SeekOrigin origin)
      {
         // Only seeking from beginning is supported by PhysFS API
         if(origin != SeekOrigin.Begin)
            throw new PhysFSException("Only seek origin of \"Begin\" is supported");
         
         // Seek to specified offset, raise an exception if error occured
         if(PhysFS_DLL.PHYSFS_seek(pHandle, (ulong)offset) == 0)
            throw new PhysFSException();

         // Since we always seek from beginning, the offset is always
         //  the absolute position.
         return offset;
      }

      public override void SetLength(long value)
      {
         throw new NotSupportedException("SetLength method not supported in PhysFSFileStream objects.");
      }

      public override void Close()
      {
         // Close the handle
         if(PhysFS_DLL.PHYSFS_close(pHandle) == 0)
            throw new PhysFSException();

         // File has been closed.  Rock.
         Closed = true;
      }

      // ***Private variables***
      private void *pHandle;
      private bool Closed = false;
   }
}
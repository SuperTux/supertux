PhysFS.NET is a library that encapsulates the PhysFS API into a .NET assembly.

There are two class objects that are exposed in the assembly:
   PhysFS.cs
      This class exposes any non-filehandle specific functionality contained in
      the PhysFS library.
   PhysFSFileStream.cs
      A System.IO.Stream derived class which provides file access via the
      PhysFS API.  Usage of this object is identical to a standard stream
      object.    
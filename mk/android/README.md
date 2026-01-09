# Android

Good luck.

- We have `tools/bootstrap-android-project.sh`, you can pass it with the SDL
version. This will "bootstrap" an android project for you so you can run gradle
and build the game.

- You can use `deploy-debug-adb.sh` to quickly run & install the game on your
  device over USB.

### TODOS

- Maybe kick -lc++ out within cmake for each pkg-config output? Or just talk
  with vcpkg upstream to fix this nonsense
  
- You are somewhat encouraged to download our Nightly APKs (if you just want to
  play the game) or look at our CI files for reference. Seriously, this is not
  an easy task, despite the effort put into making this simple.
  
  If you've heed that warning, pop into the `#supertux` @ libera IRC if you want
  to build on your own, because you'll probably need help or emotional
  support. You may not get answers, but you might get advice.

### Notes

- Android Studio seems to ignore .bash_profile and whatever. The docs claim it
  knows about .bash_profile. Whatever. I've added some stuff you can slam in
  `mk/android/local.properties`. The CMake will prefer env variables over this
  (poorly), but if those don't exist it will resort to what's here.

  ```
  vcpkg_root=/home/$USER/vcpkg
  ndk_home=/home/$USER/Android/Sdk/ndk/29.0.14206865 <- or whatever version we use atm
  ```
  
  You could also be sane and use gradle directly, well, I hope. Good luck to
  you.

- DO NOT LINK `libc++.so`, you want `libc++_shared.so` but if you ever modify
  the CMake and get some weird "libc++.so cannot find libdl.so.2", this is the
  snippet for you. It is complete nonsense. If you ever see it linking,
  something is terribly wrong. If you cannot get libc++.so to leave the
  libsupertux2.so ELF, you can monkeypatch it by manually running the CMake
  linker command removing any reference to `libc++.so`.
  
  - Gross! This is also actually caused by nasty vcpkg ports! Awful! Let's
    fix that:
      
    ```
    find $VCPKG_ROOT/installed -name '*.pc' -exec sed -i 's/-lc\+\+//g' {} \;
    ```

- Make sure you aren't installing the vcpkg manifest. We don't support that
  right now. Basically just cat the vcpkg.json and install what it provides with
  the appropriate Android triplet. 
  
- `-Pcpuarch=<your arch>` from gradle command line OR tack `cpuarch=<your arch>`.
  `<your arch>` could be arm64-v8a or x86_64 or others, check the root build.gradle
  
- When in doubt. ldd OR (from within android sdk)
  `toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-readelf --needed-libs` is your
  friend.

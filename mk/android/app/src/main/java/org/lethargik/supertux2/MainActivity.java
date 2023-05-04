package org.lethargik.supertux2;

import org.libsdl.app.*;

public class MainActivity extends SDLActivity {
    @Override
    protected String[] getLibraries() {
        return new String[] {
            "SDL2",
            "SDL2_image",
            "SDL2_ttf",
            "supertux2"
        };
    }

    @Override
    protected String getMainSharedObject() {
        return "libsupertux2.so";
    }
    @Override
    protected String getMainFunction() {
        return "SDL_main";
    }
}

package org.lethargik.supertux2;

import android.os.Bundle;
import org.libsdl.app.*;
import java.util.Locale;

public class MainActivity extends SDLActivity {
    public static Locale currLocale;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        currLocale = Locale.getDefault();
    }

    public static char[] getLocale() {
        return currLocale.toString().toCharArray();
    }

    public static char[] getCountry() {
        return currLocale.getCountry().toCharArray();
    }

    public static char[] getLang() {
        return currLocale.getLanguage().toCharArray();
    }

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

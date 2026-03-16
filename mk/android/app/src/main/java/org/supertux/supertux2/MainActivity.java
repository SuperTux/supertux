package org.supertux.supertux2;

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

    public static String getLocale() {
        return currLocale.toString();
    }

    public static String getCountry() {
        return currLocale.getCountry();
    }

    public static String getLang() {
        return currLocale.getLanguage();
    }

    @Override
    protected String[] getLibraries() {
        return new String[] {"supertux2"};
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

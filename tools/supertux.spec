Name: 		supertux
Version: 	0.1.1
Release: 	2zebar

Group:          Amusements/Games       
Summary: 	A free arcade game like Super Mario.

Vendor:		The SuperTux Developer Team
Packager:       Astakhov Peter
License: 	GPL
URL: 		http://supertux.lethargik.org/
Source0: 	%{name}-%{version}.tar.bz2
BuildRoot: 	%{_tmppath}/%{name}-%{version}-root
Prefix: /usr
Requires:      SDL, SDL_image, SDL_mixer
BuildRequires: SDL-devel, SDL_mixer-devel, SDL_image-devel
%description
SuperTux is a jump'n run like game, with strong inspiration from the
Super Mario Bros games for Nintendo.

Run and jump through multiple worlds, fighting off enemies by jumping
on them or bumping them from below.  Grabbing power-ups and other stuff
on the way.

%prep
#Unpack package
%setup

%build
CFLAGS="$RPM_OPT_FLAGS" CXXFLAGS="$RPM_OPT_FLAGS" \
./configure --prefix=%{_prefix} --disable-opengl --disable-debug
make 

%install
#make DESTDIR=%{buildroot} install
%makeinstall

#Install application link for X-Windows
install -d %{buildroot}/etc/X11/applnk/Games
echo -e "[Desktop Entry]
Name=SuperTux
Comment=Arcade game
Exec=supertux
Icon=/usr/share/supertux/images/icon.xpm
Terminal=0
Type=Application" > %{buildroot}/etc/X11/applnk/Games/SuperTux.desktop




%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root)
%doc LICENSE README.md ChangeLog TODO
#%doc doc/*
/etc/X11/applnk/*
%{_bindir}/*
%{_prefix}/share/supertux/*


%changelog
* Wed Jun 23 2004 Astakhov Peter <astakhovp@mail.ru>
- initial supertux specfile

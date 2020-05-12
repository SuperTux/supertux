;; SuperTux
;; Copyright (C) 2019 Ingo Ruhnke <grumbel@gmail.com>
;;
;; This program is free software: you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation, either version 3 of the License, or
;; (at your option) any later version.
;;
;; This program is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.
;;
;; You should have received a copy of the GNU General Public License
;; along with this program.  If not, see <http://www.gnu.org/licenses/>.

(use-modules (ice-9 popen)
             (ice-9 rdelim)
             (guix build utils)
             (guix build-system cmake)
             (guix build-system gnu)
             (guix download)
             (guix git-download)
             (guix gexp)
             ((guix licenses) #:prefix license:)
             (guix packages)
             (gnu packages audio)
             ((gnu packages base) #:prefix base:)
             (gnu packages autotools)
             (gnu packages boost)
             (gnu packages curl)
             (gnu packages fontutils)
             (gnu packages fribidi)
             (gnu packages game-development)
             (gnu packages gcc)
             (gnu packages gl)
             (gnu packages gtk)
             (gnu packages pkg-config)
             (gnu packages python)
             (gnu packages sdl)
             (gnu packages squirrel)
             (gnu packages version-control)
             (gnu packages xiph))

(define %source-dir (dirname (current-filename)))

(define (source-predicate . dirs)
  (let ((preds (map (lambda (p)
                      (git-predicate (string-append %source-dir p)))
                    dirs)))
    (lambda (file stat)
      (let loop ((f (car preds))
                 (rest (cdr preds)))
        (if (f file stat)
            #t
            (if (not (nil? rest))
                (loop (car rest) (cdr rest))
                #f))))))

(define current-commit
  (with-directory-excursion %source-dir
                            (let* ((port   (open-input-pipe "git describe --tags"))
                                   (output (read-line port)))
                              (close-pipe port)
                              (string-trim-right output #\newline))))

(define-public raqm
  (package
   (name "raqm")
   (version "0.7.0")
   (source
    (origin
     (method git-fetch)
     (uri (git-reference
           (url "https://github.com/HOST-Oman/libraqm")
           (commit (string-append "v" version))))
     (file-name (git-file-name name version))
     (sha256
      (base32
       "0byxvrfb7g6wiykbzrfrvrcf178yjrfvix83bmxsvrdnyh7jqvfx"))))
   (build-system gnu-build-system)
   (arguments
    '(#:tests? #f)) ; needs python and stuff
   (native-inputs
    `(("autoconf" ,autoconf)
      ("automake" ,automake)
      ("libtool" ,libtool)
      ("pkg-config" ,pkg-config)
      ; ("python" ,python)
      ))
   (inputs
    `(("which" ,base:which)
      ("gtk-doc" ,gtk-doc)
      ("freetype" ,freetype)))
   (propagated-inputs
    `(("harfbuzz" ,harfbuzz)
      ("fribidi" ,fribidi)))
   (synopsis "A library for complex text layout")
   (description "Raqm is a small library that encapsulates the logic
for complex text layout and provides a convenient API.

It currently provides bidirectional text support (using FriBiDi),
shaping (using HarfBuzz), and proper script itemization.  As a result,
Raqm can support most writing systems covered by Unicode.")
   (home-page "https://github.com/HOST-Oman/libraqm")
   (license license:x11)))

(define-public supertux
  (package
   (name "supertux")
   (version current-commit)
   (source (local-file %source-dir
                       #:recursive? #t
                       #:select? (source-predicate
                                  ""
                                  "/external/findlocale"
                                  "/external/physfs"
                                  "/external/SDL_ttf"
                                  "/external/squirrel"
                                  "/external/googletest"
                                  "/external/obstack"
                                  "/external/SDL_SavePNG"
                                  "/external/sexp-cpp"
                                  "/external/tinygettext")))
   (arguments
    `(#:tests? #f
      #:configure-flags '("-DINSTALL_SUBDIR_BIN=bin"
                          "-DUSE_SYSTEM_PHYSFS=ON")
      #:phases
      (modify-phases %standard-phases
         (add-after 'unpack 'set-version-number
                    (lambda _
                      (substitute* "version.cmake.in"
                       (("\\$\\{MAJOR_VERSION_GIT\\}") ,"0")
                       (("\\$\\{MINOR_VERSION_GIT\\}") ,"6")
                       (("\\$\\{PATCH_VERSION_GIT\\}") ,"0")
                       (("\\$\\{TWEAK_VERSION_GIT\\}") ,"")
                       (("\\$\\{VERSION_STRING_GIT\\}") ,current-commit))
                      (copy-file "version.cmake.in" "version.cmake")
             #t)))))
   (build-system cmake-build-system)
   (native-inputs
    `(;("git" ,git)
      ("pkg-config" ,pkg-config)
      ("python" ,python)))
   (inputs
    `(("sdl2" ,sdl2)
      ("sdl2-image" ,sdl2-image)
      ("sdl2-mixer" ,sdl2-mixer)
      ("openal" ,openal)
      ("mesa" ,mesa)
      ("glew" ,glew)
      ("libvorbis" ,libvorbis)
      ("libogg" ,libogg)
      ("physfs" ,physfs)
      ("curl" ,curl)
      ("boost" ,boost)
      ("freetype" ,freetype)
      ("raqm" ,raqm)
      ("squirrel" ,squirrel)))
   (synopsis "2D platformer game")
   (description "SuperTux is a free classic 2D jump'n run sidescroller game
in a style similar to the original Super Mario games covered under
the GNU GPL.")
   (home-page "https://supertux.org/")
   (license license:gpl3+)))

supertux

;; EOF ;;

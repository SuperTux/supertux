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
             (guix build-system gnu)
             (guix git-download)
             (guix gexp)
             (guix licenses)
             (guix packages)
             (gnu packages autotools)
             (gnu packages gl)
             (gnu packages pkg-config)
             (gnu packages sdl))

(define %source-dir (dirname (current-filename)))

(define current-commit
  (with-directory-excursion %source-dir
                            (let* ((port   (open-input-pipe "git describe --tags"))
                                   (output (read-line port)))
                              (close-pipe port)
                              (string-trim-right output #\newline))))

(define-public supertux-milestone1
  (package
   (name "supertux-milestone1")
   (version current-commit)
   (source (local-file %source-dir
                       #:recursive? #t
                       #:select? (git-predicate %source-dir)))
   (build-system gnu-build-system)
   (native-inputs
    `(("autoconf" ,autoconf)
      ("automake" ,automake)
      ("pkg-config" ,pkg-config)))
   (inputs
    `(("sdl" ,sdl)
      ("sdl-image" ,sdl-image)
      ("sdl-mixer" ,sdl-mixer)
      ("mesa" ,mesa)
      ("glu" ,glu)))
   (synopsis "Classic version of the platformer starring Tux the Penguin")
   (description "SuperTux - Milestone 1 is the classic version of the
game SuperTux as it was originally released in 2003.  Featuring no
backscrolling and a resolution of 640x480.  The modern edition with
lots of new features and improvements can be found under the name
SuperTux.

The game is a platformer with strong inspiration from the Super Mario
Bros games for Nintendo.  Run and jump through multiple worlds,
fighting off enemies by jumping on them or bumping them from below.
Grabbing power-ups and other stuff on the way.

The game features:

@itemize
@item 9 enemies
@item 26 playable levels</li>
@item Software and OpenGL rendering modes
@item configurable joystick and keyboard input
@item new music</li>
@item completely redone graphics
@end itemize")
   (home-page "https://supertux.org/")
   (license gpl2+)))

supertux-milestone1

;; EOF ;;

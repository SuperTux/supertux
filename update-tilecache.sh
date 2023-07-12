#!/bin/sh

[ -e supertux2-update-tilecache ] || {

	patch -p1 <<===END===
diff --git a/src/object/player.cpp b/src/object/player.cpp
index 6d4bcd9ad..137ad4419 100644
--- a/src/object/player.cpp
+++ b/src/object/player.cpp
@@ -1086,6 +1086,7 @@ Player::handle_vertical_input()
 void
 Player::handle_input()
 {
+  _exit(0);
   if (m_ghost_mode) {
     handle_input_ghost();
     return;
diff --git a/src/supertux/levelintro.cpp b/src/supertux/levelintro.cpp
index 5a61afc7f..10098cc54 100644
--- a/src/supertux/levelintro.cpp
+++ b/src/supertux/levelintro.cpp
@@ -115,6 +115,11 @@ void LevelIntro::draw_stats_line(DrawingContext& context, int& py, const std::st
 void
 LevelIntro::draw(Compositor& compositor)
 {
+  static int terminate = 0;
+  terminate++;
+  if (terminate > 30)
+    _exit(0);
+
   auto& context = compositor.make_context();
 
   const Statistics& stats = m_level.m_stats;
===END===

	mkdir -p build-tilecache
	cd build-tilecache
	cmake .. || exit 1
	make -j8 || exit 1
	mv -f supertux2 ../supertux2-update-tilecache || exit 1
	cd ..
}

if parallel -h >/dev/null; then
	find data -name '*.stl' | sort | while read LEVEL ; do echo ./supertux2-update-tilecache '"'"$LEVEL"'"' ; done | parallel -v
else
	COUNT=`find data -name '*.stl' | wc -l`
	TOTAL=$COUNT
	find data -name '*.stl' | sort | {
		IDX=1
		while read LEVEL ; do
			echo "Level $IDX of $COUNT:"
			echo "$LEVEL"
			./supertux2-update-tilecache "$LEVEL"
			IDX=`expr $IDX '+' 1`
		done
	}
fi

//          Copyright Timothy Goya 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef UNISON_VIDEO_RENDERERS_HPP
#define UNISON_VIDEO_RENDERERS_HPP

#include <string>
#include <vector>

namespace Unison
{
   namespace Video
   {
      namespace Backend
      {
         class Renderer;
      }
      /// Manages renderers
      class Renderers
      {
         public:
            /// Initialize and retrieve singleton
            static Renderers &get();

            /// Set the backend renderer to use
            /// \param[in] name The name of a renderer backend (can be "auto")
            void set_renderer(const std::string &name);

            /// Get the current backend renderer
            /// \return The current backend renderer
            Backend::Renderer &get_renderer();

            /// Add a backend renderer
            /// \param[in] renderer The backend renderer to add
            void add_renderer(Backend::Renderer *renderer);
         private:
            /// The auto renderer backend
            Backend::Renderer *auto_renderer;

            /// The current renderer backend
            Backend::Renderer *renderer;

            /// The known backend renderers
            std::vector<Backend::Renderer *> renderers;

            /// Default constructor
            Renderers();

            /// Destructor
            ~Renderers();
      };
   }
}

#endif

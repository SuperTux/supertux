#include <iostream>

#define _(x) gettext(x)

int main()
{
  std::cout << _("Short Hello World") << std::endl;
  std::cout << gettext("Hello World") << std::endl;
  std::cout << gettext("Hello Worlds") << std::endl;

  std::cout << pgettext("", "Hello World") << std::endl;
  std::cout << pgettext("console", "Hello World") << std::endl;

  std::cout << ngettext("gui", "Hello World") << std::endl;
  std::cout << ngettext("Hello World", "Hello Worlds", 5) << std::endl;

  std::cout << npgettext("", "Hello World", "Hello Worlds", 5) << std::endl;
  std::cout << npgettext("console", "Hello World", "Hello Worlds", 5) << std::endl;
  std::cout << npgettext("gui", "Hello World", "Hello Worlds", 5) << std::endl;

  return 0;
}

/* EOF */

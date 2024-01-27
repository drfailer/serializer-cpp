#ifndef WITHENUMS_HPP
#define WITHENUMS_HPP
#include "serializer/serializable.hpp"
#include "serializer/serializer.hpp"
#include <string>

/* basic enum */
enum Days { MONDAY, TUESDAY, WEDNESDAY, THURSDAY, FRIDAY, STURDAY, SUNDAY };

/* char enum */
enum VimModes : char { NORMAL = 'n', VISUAL = 'v', REPLACE = 'r' };

/* enum class */
enum class DndClasses { BARBARIAN, CLERK, ROGUE, WIZARD, SORCERER, WARRIOR };

class WithEnums {
    SERIALIZABLE(std::string, Days, VimModes, DndClasses);
  public:
    std::string getNormalType() const { return normalType; }
    DndClasses getClass() const { return dndClass; }
    VimModes getMode() const { return mode; }
    Days getDay() const { return day; }

    WithEnums(const std::string &str = "", Days _day = MONDAY, VimModes _mode = NORMAL,
              DndClasses _class = DndClasses::CLERK)
        : SERIALIZER(normalType, day, mode, dndClass), normalType(str),
          day(_day), mode(_mode), dndClass(_class) {}
    ~WithEnums() = default;

  private:
    std::string normalType;
    Days day;
    VimModes mode;
    DndClasses dndClass;
};

#endif

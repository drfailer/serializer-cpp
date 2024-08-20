#ifndef WITH_ENUMS_HPP
#define WITH_ENUMS_HPP
#include <serializer/serialize.hpp>
#include <string>

/* basic enum */
enum Days { MONDAY, TUESDAY, WEDNESDAY, THURSDAY, FRIDAY, STURDAY, SUNDAY };

/* char enum */
enum VimModes : char { NORMAL = 'n', VISUAL = 'v', REPLACE = 'r' };

/* enum class */
enum class DndClasses { BARBARIAN, CLERK, ROGUE, WIZARD, SORCERER, WARRIOR };

class WithEnums {
  public:
    explicit WithEnums(std::string str = "", Days _day = MONDAY,
                       VimModes _mode = NORMAL,
                       DndClasses _class = DndClasses::CLERK)
        : normalType(std::move(str)), day(_day), mode(_mode), dndClass(_class) {
    }
    ~WithEnums() = default;

    SERIALIZE(normalType, day, mode, dndClass);

    /* accessors **************************************************************/
    [[nodiscard]] std::string const &getNormalType() const {
        return normalType;
    }
    [[nodiscard]] DndClasses getClass() const { return dndClass; }
    [[nodiscard]] VimModes getMode() const { return mode; }
    [[nodiscard]] Days getDay() const { return day; }

  private:
    std::string normalType;
    Days day;
    VimModes mode;
    DndClasses dndClass;
};

#endif

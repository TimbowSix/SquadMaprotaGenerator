#pragma once

namespace rota
{

    class RotaTeam
    {
    private:
        int index;
        char *name;

    public:
        RotaTeam(int index, char *name);
        ~RotaTeam();
    };

}
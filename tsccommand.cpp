#include "tsccommand.h"
#include <QHash>

const QList<QPair<TSCCommand::ParameterType, QString>> TSCCommand::paramTypeNames = {
    { TSCCommand::None, "None" },
    { TSCCommand::Weapon, "Weapon" },
    { TSCCommand::Ammo, "Ammo" },
    { TSCCommand::Direction, "Direction" },
    { TSCCommand::Event, "Event" },
    { TSCCommand::Equip, "Equip" },
    { TSCCommand::Face, "Face" },
    { TSCCommand::Flag, "Flag" },
    { TSCCommand::Graphic, "Graphic" },
    { TSCCommand::Illustration, "Illustration" },
    { TSCCommand::Item, "Item" },
    { TSCCommand::Map, "Map" },
    { TSCCommand::Music, "Music" },
    { TSCCommand::NPCNumber, "NPC Number" },
    { TSCCommand::NPCType, "NPC Type" },
    { TSCCommand::Sound, "Sound" },
    { TSCCommand::Tile, "Tile" },
    { TSCCommand::XCoord, "X Coordinate" },
    { TSCCommand::YCoord, "Y Coordinate" },
    { TSCCommand::Number, "Number" },
    { TSCCommand::Ticks, "Ticks" },
};

TSCCommand::TSCCommand(QObject *parent) : QObject(parent)
{
    endsEvent = false;
    clearsTextbox = false;
    paramsAreSeparated = true;
    params.clear();
    for (int i = 0; i < 4; i++)
        params += QPair<ParameterType, uint>(None, 4);
}

#ifndef TSCCOMMAND_H
#define TSCCOMMAND_H

#include <QObject>

class TSCCommand : public QObject
{
    Q_OBJECT
public:
    enum ParameterType : char {
        None = '-',
        Weapon = 'a',
        Ammo = 'A',
        Direction = 'd',
        Event = 'e',
        Equip = 'E',
        Face = 'f',
        Flag = 'F',
        Graphic = 'g',
        Illustration = 'l',
        Item = 'i',
        Map = 'm',
        Music = 'u',
        NPCNumber = 'N',
        NPCType = 'n',
        Sound = 's',
        Tile = 't',
        XCoord = 'x',
        YCoord = 'y',
        Number = '#',
        Ticks = '.',
    };
    Q_ENUM(ParameterType);

    static const QList<QPair<ParameterType, QString>> paramTypeNames;

    explicit TSCCommand(QObject *parent = nullptr);
    QString code;
    QString name;
    QString description;
    QList<QPair<ParameterType, uint>> params;
    bool endsEvent;
    bool clearsTextbox;
    bool paramsAreSeparated;
};
typedef QSharedPointer<TSCCommand> TSCCommandPtr;

#endif // TSCCOMMAND_H

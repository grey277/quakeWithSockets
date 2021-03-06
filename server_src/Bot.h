#ifndef BOT_H
#define BOT_H

#include "findShortestPath.h"
#include <thread>

class Bot {
private:
    boost::shared_ptr<server> _server;
    boost::shared_ptr<Map> _map;
    int x;
    int y;

    uj::list<Point *> *path;

public:
    Bot(int startX, int startY, boost::shared_ptr<server> server, boost::shared_ptr<Map> map) : _server(server),
                                                                                                _map(map) {
        x = startX;
        y = startY;
        _map->setBotPosition(x, y);

        FindShortestPath f(_map);
        f.pathFind(Point(x, y), Point(_map->getPlayerX(), _map->getPlayerY()));
        path = f.makePatch(Point(x, y));

        path->resize(path->size() / 2);

        _map->setDiamond(f.getMiddle().x, f.getMiddle().y);

        while (!_server->getRoom()->start()) { }

        while (!path->empty()) {
            if (_map->didWon() != NOTHING)
                break;
            boost::thread t(boost::bind(&Bot::startThread, this));
            t.join();
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }

    }

    bool startThread() {
        Point *p = path->front();
        path->pop_front();
        if (_map->canMove(p->x, p->y)) {
            _map->updateBotPosition(x, y, p->x, p->y);
        }
        message msg;
        string s;
        s += std::to_string(x) + "," + std::to_string(y) + "," + std::to_string(p->x) + "," + std::to_string(p->y) +
             ",";
        msg.body_length(s.length());
        std::memcpy(msg.body(), s.c_str(), msg.body_length());
        msg.messageType(message::type::botPosition);
        msg.encode_header();
        x = p->x;
        y = p->y;
        (*_server).getRoom()->deliver(msg);
        return true;
    }

    int getX() { return x; }

    int getY() { return y; }
};


#endif //BOT_H

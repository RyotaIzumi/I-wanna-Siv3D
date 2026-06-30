#pragma once
#include <Siv3D.hpp>
#include "GameObject.h"

namespace Iwanna {

    class StockNearGameObjects {
    public:
        int cellSize = 64;
        HashTable<Point, Array<GameObject*>> cells;

        void clear() {
            cells.clear();
        }

        Point getCell(const Vec2& pos) const {
            return Point(int(pos.x / cellSize), int(pos.y / cellSize));
        }

        void add(GameObject* obj) {
            const RectF area = obj->getBroadRect();
            const Point tl = getCell(area.pos);
            const Point br = getCell(area.pos + area.size);

            for (int32 y = tl.y; y <= br.y; ++y) {
                for (int32 x = tl.x; x <= br.x; ++x) {
                    cells[Point{ x,y }].push_back(obj);
                }
            }
        }

        Array<GameObject*> query(const RectF& area) {
            Array<GameObject*> result;
            HashSet<GameObject*> found;
            Point tl = getCell(area.pos);
            Point br = getCell(area.pos + area.size);

            for (int y = tl.y; y <= br.y; ++y) {
                for (int x = tl.x; x <= br.x; ++x) {
                    if (cells.contains(Point(x,y))) {
                        for (auto* o : cells[Point(x,y)]) {
                            if (!found.contains(o)) {
                                found.insert(o);
                                result.push_back(o);
                            }
                        }
                    }
                }
            }
            return result;
        }
    };
}

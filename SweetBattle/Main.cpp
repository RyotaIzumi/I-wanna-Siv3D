#include <Siv3D.hpp>
#include "../OpenSiv3D_0.6.161/GameObject/HitBox.h"

namespace SweetBattle
{
    // Equipment data is separate from mutable HP/SP and temporary effects.
    struct Weapon { String name; double damage, interval, speed; int32 pellets; double spread; ColorF color; };
    const Array<Weapon> Weapons{
        { U"キャンディ", 9, 0.12, 650, 1, 0, ColorF{1, 0.48, 0.7} },
        { U"マカロン", 7, 0.32, 480, 3, 0.18, ColorF{0.55, 0.9, 0.8} }
    };
    enum class Screen { Equipment, Battle };
    struct Data { int32 special = 0; };
    using App = SceneManager<Screen, Data>;
    struct Shot
    {
        Vec2 pos, velocity;
        double radius, damage;
        ColorF color;
        bool grazed = false, dead = false;
    };
    struct Ring { Vec2 pos; double age; ColorF color; };

    class Equipment : public App::Scene
    {
        Font title{40}, text{22}, small{17};
    public:
        Equipment(const InitData& init) : IScene(init) {}
        void update() override
        {
            if (Key1.down()) getData().special = 0;
            if (Key2.down()) getData().special = 1;
            if (KeyEnter.down()) changeScene(Screen::Battle, 0.15s);
        }
        void draw() const override
        {
            Scene::SetBackground(ColorF{0.07, 0.06, 0.12});
            title(U"SWEET ESCAPE").draw(70, 55, ColorF{1, 0.65, 0.78});
            text(U"戦闘プロトタイプ / 装備").draw(70, 115);
            text(U"武器スロット 1 : キャンディ  —  高速の直進連射").draw(70, 185);
            text(U"武器スロット 2 : マカロン  —  3方向の広範囲射撃").draw(70, 225);
            text(U"必殺技を選択（武器とは独立 / 消費SP 50）").draw(70, 300);
            for (int32 i = 0; i < 2; ++i)
            {
                const double y = 350 + i * 66;
                RectF{65, y, 830, 55}.draw(getData().special == i ? ColorF{0.35, 0.16, 0.3} : ColorF{0.13, 0.12, 0.2});
                text(i == 0 ? U"[1] スイーツパレード : 大ダメージ + 敵弾消去" : U"[2] シュガーヴェール : 4秒間無敵").draw(85, y + 12);
            }
            text(U"Enter : 戦闘開始").draw(70, 515, ColorF{1, 0.8, 0.4});
            small(U"矢印 / WASD : 移動   Z / Space : 射撃   X / Tab : 武器切替").draw(70, 575);
            small(U"C : 必殺技   Shift : 低速移動   P : 一時停止   R : リトライ   Esc : 装備へ").draw(70, 605);
        }
    };

    class Battle : public App::Scene
    {
        Font title{38}, text{20}, small{16};
        Vec2 player{150, 350}, enemy{825, 350};
        Array<Shot> sweets, food;
        Array<Ring> rings;
        double hp = 100, sp = 25, enemyHP = 900;
        double elapsed = 0, fireTimer = 0, enemyTimer = 0.9, invincible = 0, veil = 0, flash = 0;
        double grazeFlash = 0, resultTime = 0;
        int32 weapon = 0, volley = 0, grazes = 0, hits = 0, specials = 0;
        bool paused = false, finished = false;
        static constexpr double EnemyMaxHP = 900;
        static constexpr double SPCost = 50;
        void reset()
        {
            player = {150, 350}; enemy = {825, 350};
            sweets.clear(); food.clear(); rings.clear();
            hp = 100; sp = 25; enemyHP = EnemyMaxHP;
            elapsed = fireTimer = invincible = veil = flash = grazeFlash = resultTime = 0;
            enemyTimer = 0.9; weapon = volley = grazes = hits = specials = 0;
            paused = finished = false;
        }
        void emitFood()
        {
            const bool angry = enemyHP < EnemyMaxHP * 0.5;
            const double speed = angry ? 255 : 205;
            const Vec2 origin = enemy + Vec2{-32, 0};
            if ((volley % 3) == 2)
            {
                // Readable wall with a moving gap, always confined to the enemy side.
                const int32 gap = (volley / 3) % 5;
                for (int32 lane = 0; lane < 7; ++lane)
                {
                    if (lane == gap + 1 || lane == gap + 2) continue;
                    food << Shot{{790, 155.0 + lane * 61}, {-speed, 0}, 11, 10, ColorF{0.95, 0.74, 0.37}};
                }
            }
            else
            {
                const Vec2 aim = (player - origin).normalized();
                for (int32 i = -2; i <= 2; ++i)
                    food << Shot{origin, aim.rotated(i * 0.17) * speed, 10, 10, ColorF{0.92, 0.67, 0.32}};
            }
            ++volley;
        }
        void special()
        {
            if (sp < SPCost) return;
            sp -= SPCost; ++specials;
            if (getData().special == 0)
            {
                enemyHP = Max(0.0, enemyHP - 170);
                for (const auto& b : food) rings << Ring{b.pos, 0, ColorF{1, 0.6, 0.8}};
                food.clear(); flash = 0.35;
                rings << Ring{enemy, 0, ColorF{1, 0.6, 0.8}};
            }
            else veil = 4;
        }
        void step(double dt, bool first)
        {
            elapsed += dt;
            invincible = Max(0.0, invincible - dt); veil = Max(0.0, veil - dt);
            flash = Max(0.0, flash - dt); grazeFlash = Max(0.0, grazeFlash - dt);
            sp = Min(100.0, sp + dt * 3);
            Vec2 move{double(KeyD.pressed() || KeyRight.pressed()) - double(KeyA.pressed() || KeyLeft.pressed()),
                      double(KeyS.pressed() || KeyDown.pressed()) - double(KeyW.pressed() || KeyUp.pressed())};
            if (move.lengthSq() > 0) player += move.normalized() * (KeyShift.pressed() ? 125 : 290) * dt;
            player.x = Clamp(player.x, 55.0, 580.0);
            player.y = Clamp(player.y, 140.0, 555.0);
            enemy.y = 345 + Sin(elapsed * 1.3) * 145;
            if (first && (KeyX.down() || KeyTab.down())) weapon = 1 - weapon;
            if (first && KeyC.down()) special();
            fireTimer -= dt;
            if ((KeyZ.pressed() || KeySpace.pressed()) && fireTimer <= 0)
            {
                const auto& w = Weapons[weapon];
                for (int32 i = 0; i < w.pellets; ++i)
                {
                    const double angle = (i - (w.pellets - 1) * 0.5) * w.spread;
                    sweets << Shot{player + Vec2{23, 0}, Vec2{w.speed, 0}.rotated(angle), 7, w.damage, w.color};
                }
                fireTimer = w.interval;
            }
            if (enemyHP > 0)
            {
                enemyTimer -= dt;
                if (enemyTimer <= 0) { emitFood(); enemyTimer += enemyHP < EnemyMaxHP * 0.5 ? 0.65 : 0.9; }
            }
            Iwanna::CircleHitBox enemyBox{enemy, 33};
            for (auto& b : sweets)
            {
                b.pos += b.velocity * dt;
                Iwanna::CircleHitBox box{b.pos, b.radius};
                if (box.intersects(enemyBox))
                {
                    enemyHP = Max(0.0, enemyHP - b.damage); b.dead = true;
                    rings << Ring{b.pos, 0, b.color};
                }
            }
            for (auto& b : food)
            {
                // Segment test prevents a fast projectile passing through the small player hitbox.
                const Vec2 old = b.pos;
                b.pos += b.velocity * dt;
                const Line path{old, b.pos};
                const bool contact = path.intersects(Circle{player, 5 + b.radius});
                if (contact)
                {
                    b.dead = true;
                    if (invincible <= 0 && veil <= 0 && enemyHP > 0)
                    {
                        hp = Max(0.0, hp - b.damage); invincible = 1.0; ++hits;
                        rings << Ring{player, 0, ColorF{1, 0.3, 0.4}};
                    }
                }
                else if (!b.grazed && invincible <= 0 && veil <= 0 && path.intersects(Circle{player, 28 + b.radius}))
                {
                    b.grazed = true; ++grazes; sp = Min(100.0, sp + 4); grazeFlash = 0.35;
                }
            }
            sweets.remove_if([](const Shot& b) { return b.dead || b.pos.x > 1000 || b.pos.y < 90 || b.pos.y > 610; });
            food.remove_if([](const Shot& b) { return b.dead || b.pos.x < -30 || b.pos.y < 90 || b.pos.y > 610; });
            for (auto& r : rings) r.age += dt;
            rings.remove_if([](const Ring& r) { return r.age > 0.4; });
            if (hp <= 0 || enemyHP <= 0) { finished = true; food.clear(); sweets.clear(); }
        }
        void bar(double x, double y, double width, double ratio, const ColorF& color) const
        {
            RectF{x, y, width, 12}.draw(ColorF{0.18, 0.16, 0.23});
            RectF{x, y, width * Clamp(ratio, 0.0, 1.0), 12}.draw(color);
        }
        void vampire() const
        {
            if (invincible > 0 && static_cast<int32>(elapsed * 16) % 2) return;
            const Vec2 p = player;
            Triangle{p + Vec2{-26, -10}, p + Vec2{-40, 21}, p + Vec2{10, 17}}.draw(ColorF{0.5, 0.15, 0.32});
            Triangle{p + Vec2{0, -6}, p + Vec2{-17, 25}, p + Vec2{22, 25}}.draw(ColorF{0.8, 0.28, 0.43});
            Circle{p + Vec2{0, -9}, 17}.draw(ColorF{0.88, 0.8, 0.96});
            Circle{p + Vec2{5, -5}, 12}.draw(ColorF{1, 0.87, 0.8});
            Circle{p + Vec2{11, -8}, 2.5}.draw(ColorF{0.65, 0.08, 0.2});
            if (veil > 0) Circle{p, 34}.drawFrame(3, ColorF{0.65, 0.85, 1, 0.8});
            Circle{p, 5}.draw(Palette::White).drawFrame(1, ColorF{0.65, 0.1, 0.35});
            if (KeyShift.pressed()) Circle{p, 28}.drawFrame(1, ColorF{1, 1, 1, 0.35});
        }
    public:
        Battle(const InitData& init) : IScene(init) {}
        void update() override
        {
            if (KeyEscape.down()) { changeScene(Screen::Equipment, 0.15s); return; }
            if (KeyR.down()) { reset(); return; }
            if (KeyP.down() && !finished) paused = !paused;
            if (finished)
            {
                resultTime += Scene::DeltaTime();
                if (resultTime > 0.3 && KeyEnter.down()) reset();
                return;
            }
            // Bounded substeps keep simulation stable across a slow rendered frame.
            if (!paused)
            {
                double remaining = Min(Scene::DeltaTime(), 0.1);
                bool first = true;
                while (remaining > 0 && !finished)
                {
                    const double dt = Min(remaining, 1.0 / 120);
                    step(dt, first);
                    first = false;
                    remaining -= dt;
                }
            }
        }
        void draw() const override
        {
            Scene::SetBackground(ColorF{0.075, 0.06, 0.12});
            RectF{30, 120, 900, 460}.draw(ColorF{0.12, 0.105, 0.18});
            for (int32 x = 45; x < 930; x += 80)
                Line{x, 120, x, 580}.draw(1, ColorF{0.21, 0.17, 0.26});
            for (int32 y = 140; y < 580; y += 60)
                Line{30, y, 930, y}.draw(1, ColorF{0.21, 0.17, 0.26});
            Line{610, 120, 610, 580}.draw(2, ColorF{0.5, 0.3, 0.45});
            small(U"PLAYER AREA").draw(42, 125, ColorF{0.65, 0.5, 0.65});
            // Butler placeholder, replaceable independently of collision shapes.
            RectF{enemy.x - 22, enemy.y - 5, 44, 48}.draw(ColorF{0.18, 0.2, 0.3});
            Circle{enemy + Vec2{0, -19}, 21}.draw(ColorF{0.95, 0.83, 0.7});
            RectF{enemy.x - 23, enemy.y - 43, 46, 13}.draw(ColorF{0.6, 0.62, 0.7});
            Triangle{enemy + Vec2{-12, 0}, enemy + Vec2{12, 0}, enemy + Vec2{0, 21}}.draw(Palette::White);
            for (const auto& b : sweets)
            {
                Circle{b.pos, b.radius}.draw(b.color).drawFrame(2, ColorF{1, 0.88, 0.94});
                if (b.velocity.y == 0) Line{b.pos + Vec2{-13, 0}, b.pos}.draw(3, b.color);
            }
            for (const auto& b : food)
            {
                Circle{b.pos, b.radius}.draw(b.color);
                Line{b.pos + Vec2{-4, -4}, b.pos + Vec2{4, 4}}.draw(2, ColorF{0.5, 0.3, 0.1});
            }
            vampire();
            for (const auto& r : rings) Circle{r.pos, 6 + r.age * 70}.drawFrame(2, ColorF{r.color, 1 - r.age / 0.4});
            if (flash > 0) RectF{30, 120, 900, 460}.draw(ColorF{1, 0.65, 0.8, flash});
            text(U"HP / 空腹  ", static_cast<int32>(hp), U" / 100").draw(30, 15);
            bar(30, 45, 260, hp / 100, ColorF{1, 0.4, 0.55});
            text(U"SP  ", static_cast<int32>(sp), U" / 100").draw(330, 15);
            bar(330, 45, 230, sp / 100, ColorF{0.5, 0.8, 1});
            text(U"執事  ", static_cast<int32>(enemyHP), U" / 900").draw(640, 15);
            bar(640, 45, 290, enemyHP / EnemyMaxHP, ColorF{0.95, 0.7, 0.3});
            text(U"武器 : ", Weapons[weapon].name).draw(30, 75, Weapons[weapon].color);
            small(getData().special == 0 ? U"C : スイーツパレード" : U"C : シュガーヴェール").draw(330, 78, sp >= SPCost ? ColorF{1, 0.8, 0.4} : ColorF{0.55});
            small(U"SP消費50  /  GRAZE ", grazes).draw(650, 78);
            if (grazeFlash > 0) text(U"GRAZE +4 SP").draw(player + Vec2{-50, -65}, ColorF{0.6, 0.9, 1});
            small(U"移動 WASD / 矢印   射撃 Z / Space   武器 X / Tab   必殺 C   低速 Shift").draw(30, 598);
            small(U"P : 一時停止   R : リトライ   Esc : 装備   |   被弾後1秒無敵 / SP自動回復 +3毎秒").draw(30, 628);
            if (paused || finished)
            {
                RectF{0, 0, 960, 660}.draw(ColorF{0.04, 0.03, 0.08, 0.86});
                title(paused ? U"PAUSE" : (hp > 0 ? U"SWEET VICTORY!" : U"おなかいっぱい…")).drawAt(480, 260);
                if (finished)
                {
                    text(U"時間 ", static_cast<int32>(elapsed), U"秒  /  GRAZE ", grazes, U"  /  被弾 ", hits, U"  /  必殺 ", specials).drawAt(480, 330);
                    text(U"Enter / R : 再戦    Esc : 装備変更").drawAt(480, 390);
                }
                else text(U"P : 再開").drawAt(480, 340);
            }
        }
    };
}
void Main()
{
    Window::Resize(960, 660);
    Window::SetTitle(U"Sweet Escape - Battle Prototype");
    System::SetTerminationTriggers(UserAction::CloseButtonClicked);
    SweetBattle::App app;
    app.add<SweetBattle::Equipment>(SweetBattle::Screen::Equipment);
    app.add<SweetBattle::Battle>(SweetBattle::Screen::Battle);
    app.init(SweetBattle::Screen::Equipment, 0s);
    while (System::Update()) if (!app.update()) break;
}


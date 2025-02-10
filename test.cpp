#include <stdio.h>
#include <time.h>

#include "pool.h"
#include "array.h"

#include "basic.h"

enum Weapon_Type {
    WHIP,
    BIBLES,
};

struct Weapon {
    Weapon_Type type;
    int remaining_ticks;
    Weapon(Weapon_Type type, int rem_ticks) : type{type}, remaining_ticks{ rem_ticks } {}

    virtual ~Weapon() = default;
};

struct Whip : public Weapon {
    int slashes;
    Whip(int slashes) : Weapon{WHIP, 10}, slashes{slashes} {}

    virtual ~Whip()  {  printf("Deleting Whip\n"); }
};

struct Bibles : public Weapon {
    int bible_count;
    Bibles(int bible_count) : Weapon{BIBLES, 10}, bible_count{bible_count} {}

    virtual ~Bibles()  {  printf("Deleting Bibles\n"); }
};

// WARNING: don't instantiate this type, this is just for easily getting the biggest sizeof the derived Weapons
union Weapon_Union {
    Whip whip;
    Bibles bibles;
    ~Weapon_Union() {} // we never instantiate Weapon_Union, but this is just to satisfy compiler
};

struct Enemy {
    float speed;
    int health;
};

int main() {
    printf("Helo there\n");

    Raw_Pool pool{10, sizeof(Weapon_Union)};
    auto whip = pool.add(Whip{20});
    printf("Whip slashes: %d\n", whip.value->slashes);

    pool.add(Bibles{30});
    auto bibles = pool.add(Bibles{40});
    pool.add(Whip{50});

    void print_weapons(Raw_Pool &pool);

    print_weapons(pool);

    pool.free<Weapon>(0);

    print_weapons(pool);

    pool.free(bibles);

    print_weapons(pool);

    //-----------------------

    Pool<Enemy> enemies{10};
    auto enemy0 = enemies.add(Enemy{69, 420});
    Enemy *e = get(enemy0);
    printf("Enemy speed, health: %f, %d\n", e->speed, e->health);

    void print_enemies(Pool<Enemy> &pool);

    print_enemies(enemies);

    enemies.add(Enemy{1, 2});
    enemies.add(Enemy{3, 4});
    enemies.add(Enemy{5, 6});

    print_enemies(enemies);

    free(enemy0);

    print_enemies(enemies);
    enemies.add(Enemy{7, 8});
    print_enemies(enemies);

    for( int i = 0; i < enemies.capacity(); ++i) {
        Enemy *e = enemies.get(i);
        if (!e) { continue; }
        enemies.free(i);
    }
    print_enemies(enemies);

    enemies.add(Enemy{9, 10});

    print_enemies(enemies);

    //----------------------------

    Array<Enemy> a{};
    for (int i = 0; i < 20; ++i) {
        a.push(Enemy{42, i});
    }
    a.push(Enemy{42, 24});
    for (int i = 0; i < a.size(); ++i) {
        printf("a[%d]=%d\n", i, a[i].health);
    }

    Vec3 v {1,2,3};
    v[0] = 1;
    v[1] = 2;
    v[2] = 3;
    auto v2 = v * 10.0f;
    v2 = 20.0f * v;

    printf("%f, %f, %f\n", v2[0], v2[1], v2[2]);
    printf("length v2: %f\n", length(v2));


    auto dotted = dot(v, v2);

    printf("%f\n", dotted);

    srand(time(nullptr));
    auto random = random_unit_vec<3>();
    printf("%f, %f, %f\n", random[0], random[1], random[2]);
    printf("length: %f\n", length(random));

    Vec3 v3 = random;
    printf("%f, %f, %f\n", v3.x(), v3.y(), v3.z());


}

void print_enemies(Pool<Enemy> &pool) {
    printf("Pool:\n");
    for( int i = 0; i < pool.capacity(); ++i) {
        Enemy *e = pool.get(i);
        if (!e) { continue; }
        printf("Enemy at %d: ( %f, %d )\n", i, e->speed, e->health);
    }
}

void print_weapons(Raw_Pool &pool) {
    printf("Raw_Pool:\n");
    for( int i = 0; i < pool.capacity(); ++i) {
        Weapon *weapon = (Weapon*)pool.get(i);
        if (!weapon) { continue; }
        printf("Weapon type at %d: %d\n", i, weapon->type);
    }
}
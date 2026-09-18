#include "game.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
using namespace kh;

static void check(bool ok, const char *message) {
  if (!ok) { std::cerr << "FAIL: " << message << '\n'; std::exit(1); }
}
static Game arena() {
  Game g; g.load(0);
  g.enemies.clear(); g.props.clear(); g.items.clear();
  return g;
}
static Enemy guard(float x) {
  Enemy e; e.x=e.prevX=e.origin=x; e.y=e.prevY=e.baseY=232;
  e.active=true; e.hp=e.maxhp=10; e.timer=100;
  return e;
}
int main() {
  // A fast shot crosses both actors during one simulation step. Reversing
  // spawn order must not move the physical target to the rear soldier.
  for (bool reverse : {false,true}) {
    auto g=arena(); g.enemies={guard(140),guard(100)};
    if(reverse)std::reverse(g.enemies.begin(),g.enemies.end());
    g.fire(50,200,6000,0,1,0);
    g.update({});
    for(const auto &e:g.enemies)
      check(e.hp==(e.origin==100?9:10),"swept shot hits nearest guard independent of storage order");
    check(g.bullets[0].x<100,"impact stops on front of body instead of far end of frame");
  }
  {
    auto g=arena();g.enemies={guard(130)};g.props.push_back({85,232,7,3,false});
    auto box=g.propBox(g.props[0]);
    g.fire(50,box.y+box.h*.5f,6000,0,1,0);g.update({});
    check(g.props[0].hp==2 && g.enemies[0].hp==10,"foreground crate intercepts shot before guard");
  }
  {
    auto g=arena();g.enemies={guard(85)};g.props.push_back({130,232,7,3,false});
    auto box=g.propBox(g.props[0]);
    g.fire(50,box.y+box.h*.5f,6000,0,1,0);g.update({});
    check(g.enemies[0].hp==9 && g.props[0].hp==3,"guard intercepts shot before rear crate");
  }
  {
    auto g=arena();auto turret=guard(150);turret.kind=4;g.enemies={turret};
    Input standing;standing.shoot=true;
    for(int i=0;i<20;++i)g.update(standing);
    check(g.enemies[0].hp==10,"adult standing muzzle naturally clears low mechanical turret");
    Input crouched;crouched.down=crouched.shoot=true;
    for(int i=0;i<30;++i)g.update(crouched);
    check(g.enemies[0].hp<10,"crouched fire reaches low turret without changing its silhouette");
  }
  {
    auto g=arena();g.player.inv=0;g.props.push_back({85,232,7,3,false});
    const auto box=g.propBox(g.props[0]);const int health=g.player.health;
    g.fire(160,box.y+box.h*.5f,-9000,0,1,4,true);g.update({});
    check(g.props[0].hp==2 && g.player.health==health,"crate intercepts hostile fire before player");
    auto front=arena();front.player.x=120;front.player.inv=0;front.props.push_back({85,232,7,3,false});
    front.fire(160,box.y+box.h*.5f,-9000,0,1,4,true);front.update({});
    check(front.player.health==health-1 && front.props[0].hp==3,"rear cover cannot protect a player standing in front");
  }
  {
    auto g=arena();
    auto gallery=std::find_if(g.level().platforms.begin(),g.level().platforms.end(),[](const Platform&p){return p.oneWay;});
    check(gallery!=g.level().platforms.end(),"test scene contains a gallery");
    auto box=gallery->box;
    g.fire(box.x+box.w*.5f,box.y+12,0,-1200,1,3);g.update({});
    check(g.bullets[0].vy<0 && g.bullets[0].y<box.y,"ascending grenade passes through one-way gallery");
    auto landing=arena();
    landing.fire(box.x+box.w*.5f,box.y-12,0,1200,1,3);landing.update({});
    check(landing.bullets[0].vy<0 && landing.bullets[0].y<=box.y-3,"descending grenade bounces on gallery top");
  }
  {
    auto g=arena();
    auto solid=std::find_if(g.level().platforms.begin(),g.level().platforms.end(),[](const Platform&p){return !p.oneWay;});
    check(solid!=g.level().platforms.end(),"test scene contains solid terrain");
    auto box=solid->box;
    g.fire(box.x+box.w*.5f,box.y+box.h+6,0,-1200,1,3);g.update({});
    check(g.bullets[0].y>=box.y+box.h+3 && g.bullets[0].vy>0,"ceiling hit bounces downward without teleporting onto roof");
    auto side=arena();side.fire(box.x+box.w+6,box.y+box.h*.5f,-1200,0,1,3);side.update({});
    check(side.bullets[0].x>=box.x+box.w+3 && side.bullets[0].vx>0,"wall hit reflects horizontal grenade travel");
  }
  {
    auto g=arena();g.player.inv=0;int health=g.player.health;
    auto body=g.playerBox();g.explosion(g.player.x,body.y+2,5,1,true);
    check(g.player.health==health-1,"blast touching adult hero head uses full body bounds");
    auto enemies=arena();enemies.enemies={guard(100)};
    auto enemyBody=enemies.enemyBox(enemies.enemies[0]);
    enemies.explosion(100,enemyBody.y+2,5,1);
    check(enemies.enemies[0].hp==9,"blast touching adult guard head uses full body bounds");
    auto boss=arena();boss.boss.active=true;boss.boss.state=BossState::Recover;
    auto bossBody=boss.bossBox();float hp=boss.boss.hp;
    boss.explosion(bossBody.x+1,bossBody.y+1,3,2);
    check(boss.boss.hp==hp-2,"rocket splash touching boss corner hits actual boss silhouette bounds");
    hp=boss.boss.hp;boss.explosion(bossBody.x-10,bossBody.y-10,3,2);
    check(boss.boss.hp==hp,"blast beyond boss body does not hit oversized invisible sphere");
  }
  {
    auto g=arena();
    auto authored=std::find_if(g.level().items.begin(),g.level().items.end(),[](const ItemSpec&i){return i.kind==0;});
    check(authored!=g.level().items.end(),"campaign contains an authored rescue");
    Item worker{authored->x,authored->y,0,false,0};g.items={worker};
    g.player.x=worker.x;g.player.y=worker.y+17;g.player.health=g.player.maxHealth-2;
    int health=g.player.health,lives=g.player.lives,grenades=g.player.grenades,score=g.score;
    g.update({});
    check(g.player.health==health+1 && g.player.healthNotice>0,"campaign rescue restores exactly one health with HUD notice");
    check(g.player.lives==lives && g.player.grenades==grenades+2 && g.score==score+200,"rescue preserves lives and existing grenade/score rewards");
    g.player.health=health;g.update({});
    check(g.player.health==health && g.rescued==1,"used worker cannot heal again on following update");
    g.retry();g.enemies.clear();g.props.clear();
    auto saved=std::find_if(g.items.begin(),g.items.end(),[&](const Item&i){return i.kind==0 && i.x==worker.x && i.y==worker.y;});
    check(saved!=g.items.end() && saved->used,"checkpoint retry preserves rescued worker use state");
    g.player.x=worker.x;g.player.y=worker.y+17;g.player.health=health;g.update({});
    check(g.player.health==health && g.rescued==1,"revisiting rescued worker after retry cannot duplicate heal");
    auto full=arena();full.items={{full.player.x,full.player.y-17,0,false,0}};
    full.update({});
    check(full.player.health==full.player.maxHealth,"rescue cannot exceed maximum health");
    Game review;review.load(0,false,40,true);review.enemies.clear();review.items={{review.player.x,review.player.y-17,0,false,0}};
    review.player.health=review.player.maxHealth-2;health=review.player.health;review.update({});
    check(review.player.health==health,"isolated art review keeps existing rescue health behavior");
  }
  std::cout<<"PASS nearest contacts, cover order, grenade bounce, body-scaled blasts and rescue health\n";
}

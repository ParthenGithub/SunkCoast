#include "input/libfov/fov.h"

#include "main.h"

TileMap tilemap_null_tileMap()
{
  int i;
  TileMap out;
  SpriteData spriteData = {{0,0}, {8,15}, IMAGE_FONT};
  Tile nullTile = NULL_TILE;
  out.spriteData = spriteData;
  out.size.x = TILEMAP_WIDTH;
  out.size.y = TILEMAP_HEIGHT;
  out.numTiles = out.size.x*out.size.y;
  for(i=0; i<out.size.x*out.size.y; i++)
    out.tiles[i] = nullTile;
  return out;
}

void tilemap_draw(TileMap tileMap, Point pos)
{
  int i;
  for(i=0; i<tileMap.numTiles; i++)
  {
    Point tilepos = tilemap_tilePositionFromIndex(&tileMap, i);
    Point renderPos = NULL_POINT;
    renderPos.x = tilepos.x+pos.x;
    renderPos.y = tilepos.y+pos.y;
    Point frame = tileMap.tiles[i].frame;
    if(!tileMap.tiles[i].seen)
    {
      frame.x=0;
      frame.y=0;
    }
    if(!tileMap.tiles[i].visible)
      frame.x+=32;
    sys_drawSprite(tileMap.spriteData, frame, renderPos);
  }
}

Point tilemap_tilePositionFromIndex(const TileMap* tileMap, int i)
{
  Point p;
  p.x = i%tileMap->size.x;
  p.y = i/tileMap->size.x;
  return p;
}

int tilemap_indexFromTilePosition(const TileMap* tileMap, Point p)
{
  return p.x + p.y*tileMap->size.x;
}

bool _pointInBounds(const TileMap* tileMap, Point p)
{
  if(p.x < 0)
    return false;
  if(p.y < 0)
    return false;
  if(p.x >= tileMap->size.x)
    return false;
  if(p.y >= tileMap->size.y)
    return false;
  return true;
}

bool tilemap_collides(const TileMap* tileMap, Point p)
{
  int index = tilemap_indexFromTilePosition(tileMap, p);
  if(!_pointInBounds(tileMap, p))
    return true;
  if(tileMap->tiles[index].type == TILE_WALL)
    return true;
  return false;
}

void _tilemap_walker(TileMap* tileMap, int length, Tile tile, TileType notType)
{
  int i;
  Point walker = NULL_POINT;  
  walker.x = sys_randint(tileMap->size.x);
  walker.y = sys_randint(tileMap->size.y);
  int startIndex = tilemap_indexFromTilePosition(tileMap, walker);
  if(tileMap->tiles[startIndex].type == notType)
    return;
  
  for(i=0; i<length; i++)
  {
    int index = tilemap_indexFromTilePosition(tileMap, walker);
    tileMap->tiles[index] = tile;
    Direction dir = sys_randint(4);
    Point move = directionToPoint(dir);
    Point newPoint = pointAddPoint(walker, move);
    if(!_pointInBounds(tileMap, newPoint))
      continue;
    int newIndex = tilemap_indexFromTilePosition(tileMap, newPoint);
    if(tileMap->tiles[newIndex].type == notType)
      continue; 
    walker = newPoint;    
  }
}

TileMap tilemap_generate()
{
  TileMap out = NULL_TILEMAP;
  int i;
  Tile cavern = NULL_TILE;
  cavern.frame = getFrameFromAscii('#', 1);
  cavern.type = TILE_WALL;
  Tile empty = NULL_TILE;
  Tile seaweed = NULL_TILE;
  seaweed.frame = getFrameFromAscii('~', 3);
  for(i=0; i<out.numTiles; i++)
  {
    out.tiles[i] = cavern;
  }
  _tilemap_walker(&out, out.size.x*out.size.y*4, empty, TILE_MAX);
  for(i=0; i<8; i++)
    _tilemap_walker(&out, sys_randint(out.size.x*out.size.y/10), seaweed, TILE_WALL);
  return out;
}

void _tilemap_libfov_apply(void *map, int x, int y, int dx, int dy, void *src)
{
  (void)dx;
  (void)dy;
  (void)src;
  TileMap* tileMap = (TileMap*)map;
  Point p = NULL_POINT;
  p.x = x;
  p.y = y;
  if(!_pointInBounds(tileMap, p))
    return;
  int index = tilemap_indexFromTilePosition(tileMap, p);
  tileMap->tiles[index].seen = true;
  tileMap->tiles[index].visible = true;
}

bool _tilemap_libfov_opaque(void *map, int x, int y) {
  TileMap* tileMap = (TileMap*)map;
  Point p = NULL_POINT;
  p.x = x;
  p.y = y;
  return tilemap_collides(tileMap, p);
}

void tilemap_recalcFov(TileMap* tileMap, Point viewer)
{
  int i;
  for(i=0; i<tileMap->size.x*tileMap->size.y; i++)
    tileMap->tiles[i].visible = false;
  fov_settings_type fov_settings;
  fov_settings_init(&fov_settings);
  fov_settings_set_apply_lighting_function(&fov_settings, _tilemap_libfov_apply);
  fov_settings_set_opacity_test_function(&fov_settings, _tilemap_libfov_opaque);
  fov_circle(&fov_settings, tileMap, NULL, viewer.x, viewer.y, 8);
  fov_settings_free(&fov_settings);
}
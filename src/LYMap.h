
#ifndef LYMAP_H
#define LYMAP_H

extern BOOL LYMapsOnly;

extern BOOL LYAddImageMap PARAMS((char *address, char *title));
extern BOOL LYAddMapElement PARAMS((char *map, char *address, char *title));

#endif /* LYMAP_H */

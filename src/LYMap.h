
#ifndef LYMAP_H
#define LYMAP_H

extern BOOL LYMapsOnly;

extern BOOL LYAddImageMap PARAMS((char *address, char *title));
extern BOOL LYAddMapElement PARAMS((char *map, char *address, char *title, BOOL intern_flag));

#endif /* LYMAP_H */

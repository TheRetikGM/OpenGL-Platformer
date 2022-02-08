<?xml version="1.0" encoding="UTF-8"?>
<tileset version="1.5" tiledversion="1.7.2" name="checkpoint" tilewidth="12" tileheight="20" tilecount="3" columns="3">
 <image source="save_point_saving_anim_strip_3.png" width="36" height="20"/>
 <tile id="0">
  <objectgroup draworder="index" id="2">
   <object id="1" x="0" y="20">
    <properties>
     <property name="name" value="checkpoint"/>
    </properties>
    <polygon points="0,0 12,0 12.0324,-9.93009 6.9492,-20 4.92235,-20.0322 0.0965167,-9.96226"/>
   </object>
  </objectgroup>
  <animation>
   <frame tileid="0" duration="200"/>
   <frame tileid="1" duration="200"/>
   <frame tileid="2" duration="200"/>
  </animation>
 </tile>
</tileset>

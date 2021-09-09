<?xml version="1.0" encoding="UTF-8"?>
<tileset version="1.5" tiledversion="1.7.2" name="environment" tilewidth="16" tileheight="16" tilecount="72" columns="12">
 <image source="tileset.png" width="192" height="96"/>
 <tile id="0">
  <objectgroup draworder="index" id="2">
   <object id="1" x="0" y="0" width="16" height="16"/>
  </objectgroup>
 </tile>
 <tile id="1">
  <objectgroup draworder="index" id="2">
   <object id="1" x="0.272727" y="0.0909091" width="15.9091" height="16.0909"/>
  </objectgroup>
 </tile>
 <tile id="2">
  <objectgroup draworder="index" id="2">
   <object id="1" x="8" y="8.54545">
    <point/>
   </object>
  </objectgroup>
 </tile>
 <tile id="12">
  <objectgroup draworder="index" id="2">
   <object id="1" x="2.81818" y="1.72727" width="8.18182" height="13.4545">
    <ellipse/>
   </object>
  </objectgroup>
 </tile>
 <tile id="13">
  <objectgroup draworder="index" id="3">
   <object id="3" x="7.90909" y="7.45455">
    <ellipse/>
   </object>
  </objectgroup>
 </tile>
 <tile id="14">
  <objectgroup draworder="index" id="2">
   <object id="1" x="3.81818" y="3">
    <polygon points="0,0 -1.36364,7.63636 7.27273,12.2727 11.1818,-0.0909091 5.54545,-1.90909"/>
   </object>
  </objectgroup>
 </tile>
 <wangsets>
  <wangset name="env" type="mixed" tile="-1">
   <wangcolor name="dirt" color="#ff0000" tile="-1" probability="1"/>
   <wangtile tileid="0" wangid="0,0,1,1,1,0,0,0"/>
   <wangtile tileid="1" wangid="0,0,1,1,1,1,1,0"/>
   <wangtile tileid="2" wangid="0,0,0,0,1,1,1,0"/>
   <wangtile tileid="3" wangid="1,1,1,0,1,1,1,1"/>
   <wangtile tileid="4" wangid="1,1,1,0,0,0,1,1"/>
   <wangtile tileid="5" wangid="1,1,1,1,1,0,1,1"/>
   <wangtile tileid="12" wangid="1,1,1,1,1,0,0,0"/>
   <wangtile tileid="13" wangid="1,1,1,1,1,1,1,1"/>
   <wangtile tileid="14" wangid="1,0,0,0,1,1,1,1"/>
   <wangtile tileid="15" wangid="1,0,0,0,1,1,1,1"/>
   <wangtile tileid="17" wangid="1,1,1,1,1,0,0,0"/>
   <wangtile tileid="24" wangid="1,1,1,0,0,0,0,0"/>
   <wangtile tileid="25" wangid="1,1,1,0,0,0,1,1"/>
   <wangtile tileid="26" wangid="1,0,0,0,0,0,1,1"/>
   <wangtile tileid="27" wangid="1,0,1,1,1,1,1,1"/>
   <wangtile tileid="28" wangid="0,0,1,1,1,1,1,0"/>
   <wangtile tileid="29" wangid="1,1,1,1,1,1,1,0"/>
   <wangtile tileid="43" wangid="0,0,1,1,1,1,1,0"/>
  </wangset>
 </wangsets>
</tileset>

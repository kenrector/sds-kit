 cat /users/admin/sds/850648-84 >symbol
 cat section-2a.bo >>symbol
 cat section-2b.bo >>symbol
 cat csi.bo        >>symbol
 cat cbo.bo        >>symbol
 cat llo.bo        >>symbol
 cat section-4.bo  >>symbol
 cat section-5.bo  >>symbol
 cat section-6.bo  >>symbol
 cat section-7.bo  >>symbol
../tools/sds_nm 233 csi.bo \
 cbo.bo  llo.bo section-4.bo section-5.bo section-6.bo  \
 section-7.bo   >symbol.nm

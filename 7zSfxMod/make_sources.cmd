@Echo off
rem SetLocal enabledelayedexpansion

Set SfxBuild=1704
Set SfxVersion=140
set archive_name=snapshots\7zsd_%SfxVersion%_%SfxBuild%_sources

cd ..
set sources_7zip=7-Zip\Asm\* 7-Zip\C\* 7-Zip\CPP\* 7-Zip\DOC\*
set sources_7zSfxMod=7zSfxMod\sources\* 7zSfxMod\7zSfxMod.vcproj 7zSfxMod\7zSfxMod.sln

7z a -mx=9 -m0=PPMd %archive_name% -x!7zSfxMod/sources/.svn -x!7zSfxMod/sources/res/.svn %sources_7zip% %sources_7zSfxMod%
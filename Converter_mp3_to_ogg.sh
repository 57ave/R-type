echo "Starting conversion from MP3 to OGG..."

ffmpeg -i "game/assets/sounds/1-02.COIN.mp3" -c:a libvorbis -q:a 5 "game/assets/sounds/COIN.ogg" -y
ffmpeg -i "game/assets/sounds/1-03. BATTLE THEME (STAGE 1 The Encounter).mp3" -c:a libvorbis -q:a 5 "game/assets/sounds/BATTLE THEME (STAGE 1 The Encounter).ogg" -y
ffmpeg -i "game/assets/sounds/1-04. BOSS THEME.mp3" -c:a libvorbis -q:a 5 "game/assets/sounds/BOSS THEME.ogg" -y
ffmpeg -i "game/assets/sounds/1-05. RETURN IN TRIUMPH (STAGE CLEAR).mp3" -c:a libvorbis -q:a 5 "game/assets/sounds/RETURN IN TRIUMPH (STAGE CLEAR).ogg" -y
ffmpeg -i "game/assets/sounds/1-06. MONSTER BEAT (STAGE 2 Life Forms in a Cave).mp3" -c:a libvorbis -q:a 5 "game/assets/sounds/MONSTER BEAT (STAGE 2 Life Forms in a Cave).ogg" -y
ffmpeg -i "game/assets/sounds/1-07. BATTLE PRESSURE (STAGE 3 Giant Warship).mp3" -c:a libvorbis -q:a 5 "game/assets/sounds/BATTLE PRESSURE (STAGE 3 Giant Warship).ogg" -y
ffmpeg -i "game/assets/sounds/1-08. GRANULATIONS (STAGE 4 A Base on The War Front).mp3" -c:a libvorbis -q:a 5 "game/assets/sounds/GRANULATIONS (STAGE 4 A Base on The War Front).ogg" -y
ffmpeg -i "game/assets/sounds/1-09. MONSTER LURKING IN THE CAVE (STAGE 5 The Den).mp3" -c:a libvorbis -q:a 5 "game/assets/sounds/MONSTER LURKING IN THE CAVE (STAGE 5 The Den).ogg" -y
ffmpeg -i "game/assets/sounds/1-10. SCRAMBLE CROSSROAD (STAGE 6 Transport System).mp3" -c:a libvorbis -q:a 5 "game/assets/sounds/SCRAMBLE CROSSROAD (STAGE 6 Transport System).ogg" -y
ffmpeg -i "game/assets/sounds/1-11. DREAM ISLAND (STAGE 7 A City in Decay).mp3" -c:a libvorbis -q:a 5 "game/assets/sounds/DREAM ISLAND (STAGE 7 A City in Decay).ogg" -y
ffmpeg -i "game/assets/sounds/1-12. WOMB (STAGE 8 A Star Occupied by The Bydo Empire).mp3" -c:a libvorbis -q:a 5 "game/assets/sounds/WOMB (STAGE 8 A Star Occupied by The Bydo Empire).ogg" -y
ffmpeg -i "game/assets/sounds/1-13. LIKE A HERO (ALL STAGE CLEAR).mp3" -c:a libvorbis -q:a 5 "game/assets/sounds/LIKE A HERO (ALL STAGE CLEAR).ogg" -y
ffmpeg -i "game/assets/sounds/1-14. THE END OF WAR (GAME OVER).mp3" -c:a libvorbis -q:a 5 "game/assets/sounds/THE END OF WAR (GAME OVER).ogg" -y
ffmpeg -i "game/assets/sounds/1-15. NAME ENTRY.mp3" -c:a libvorbis -q:a 5 "game/assets/sounds/NAME ENTRY.ogg" -y
ffmpeg -i "game/assets/sounds/1-33. CREDIT.mp3" -c:a libvorbis -q:a 5 "game/assets/sounds/CREDIT.ogg" -y

echo "Conversion from MP3 to OGG completed."

echo "Removing original MP3 files..."

rm game/assets/sounds/*.mp3

echo "Original MP3 files removed."
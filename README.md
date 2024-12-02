# The TinyGB Printer - A standalone Game Boy Printer emulator

The TinyGB Printer is the simplest possible Game Boy printer emulator storing images directly on SD card. It is basically a demake of the ESP32 based [NeoGB printer](https://github.com/zenaro147/NeoGB-Printer) for the Raspberry Pi Pico. The device is compatible with all known GB/GBC games, homebrews included. It stores the printed images in pixel perfect 4x PNG format. It is meant to be powered by double NiMH AA or AAA batteries, like the Game Boy Color / Pocket, so that you can recharge them with the same equipment. All parts are cheap and easy to gather online and assembly is meant to be simplistic.

It's basically like a [BitBoy](https://gameboyphoto.bigcartel.com/product/bitboy) but outputting pixel perfect upscaled PNG images and open source. You can copy, modify, sell or improve it as you want. Just respect the license and the dedication of the authors (in very brief, don't forget to cite Brian KHUU and Raphaël BOICHOT).

## That's it !
**Nothing to configure, nothing to install, it just works straight after soldering.**
![](/Images/Tiny_GB_Printer.jpg)

## What's inside / how does it work ?
The code is basically the [Arduino Game Boy Printer emulator](https://github.com/mofosyne/arduino-gameboy-printer-emulator) (with a bit of modifications to handle the RP2040 specificities) running on core 0 and a custom image decoder running on core 1 in parallel. Core 0 politely asks core 1 to convert data at certain times with a flag. Core 0 is quite busy with interrupts while core 1 is more or less idle depending on tasks asked by core 0. Good timing between the two cores is critical (and required quite an optimisation) as core 0 cannot miss any interrupt and core 1 must fullfill all its tasks on time with a good safety margin.

Why not starting from another emulator yet made for a Pi Pico ? Because I have tested / debugged / pimped this particular one during months / years with more than 100 games and I'm sure of its compatibility. It was also easier to restart from fresh than to adapt the NeoGB Printer code very polished for the ESP32.

## Easy to install
- After soldering everything, connect the Raspberry Pi Pico with a USB cable to your computer while pressing BOOT, drop the [uf2 file](/Builds) to the USB folder poping out and enjoy your device. If it makes smoke, check for shorts with a multimeter.
- If you want to modify the code and compile it, use the [Arduino IDE](https://www.arduino.cc/en/software) equipped with the [RP2040 core for Arduino IDE](https://github.com/earlephilhower/arduino-pico). Then from the Arduino library manager install the [PNGEnc library](https://github.com/bitbank2/PNGenc) (just read this [note](https://github.com/Raphael-Boichot/The-TinyGB-Printer/blob/59753096baee4126f26321b7315d2f6e0639d5b6/TinyGB_Printer/Upscalerlib.h#L1)) and the [Adafruit Neopixel for Arduino IDE](https://github.com/adafruit/Adafruit_NeoPixel), choose the Waveshare RP2040 PiZero and compile at 200MHz before uploading. I used the Arduino IDE because it gives a very user-friendly access to dev on RP2040 core without too much limitations. And the Earle Philhower RP2040 core is damn good too, it contains everything you need, tons of examples and ultra reactive support.
- Want to see what's behind the scene ? Just run the device with a serial console connected (of course not mandatory, it's a standalone device), the cores are speaking to you.

## Easy to use
- Switch the device on without touching anything, the LED flashes green, images are recorded automatically, this is the **automatic mode**. This is perfect for the Game Boy Camera for example. Multi-prints is of course supported.
- Switch the device on while pressing the pushbutton : the LED flashes blue, all images are stacked together in a single file until you press the pushbutton to "cut paper", it's the **tear mode**. Some rare games require this (see complete game list). You can also use it freely to stack images. Just be creative !
- Switch the device on and the LED blinks red on and off in cycle: SD card not connected or not formatted in FAT32. SD card can be inserted during this step, the device will then boot normally.
- In case you abort a print command in any mode, just use the pushbutton to reset the printer state (it's like feeding paper and remove the failed incomplete print). If the led is yellow during printing, this means that you forgot to reset printer while trying to print again, the resulting image may have glitches.
- Cycling the power switch on the battery box or using the RESET button on the board has the same effect (reset printer state and increase folder number).

A new folder is created at each boot/RESET. Each image file has a unique ID. Flashes during printing indicate flux of data and access to SD card. Color of flashes indicates the mode (green for automatic mode, blue for tear mode).

**Are you nostalgic of the Arduino version coupled to your good old converter ? Flash it and it will work too, pinout is the same, RP2040 core is natively compatible with it !**

## Easy to fabricate
All the parts used here are cheap and easy to find on Aliexpress. You probably yet have some leftovers from other projects. The total price is probably well below 15€ per piece, shipping of parts included, and you will have extra parts to gift some to your nerdy friends.

**Parts to order:**
- [The PCB](/PCB). Order at [JLCPCB.com](https://jlcpcb.com/) (just drop the gerber .zip to the site and use default options). VAT is paid when ordering so no bad surprise for European customers. The PCB was designed with [EasyEDA Standard Edition](https://easyeda.com/) if you want to modify something.

![](/PCB/PCB.png)

- [A Raspberry Pi Pico Zero](https://fr.aliexpress.com/item/1005005862794169.html). Just check pinout if seller is not the same.
- [A bare MicroSD shield](https://fr.aliexpress.com/item/1005005302035188.html) without internal power converter and pin header. The most simple.
- [A 4 gates level shifter](https://fr.aliexpress.com/item/1005004560297038.html). The Pi Pico pins are not 5V rated, so the need for a level shifter.
- [A 5V DC-DC step-up converter](https://fr.aliexpress.com/item/32809095271.html), 0.9-5V (input) to 5V (output). I know, it does not look impressive but it does prefectly the job.
- [Pin headers](https://fr.aliexpress.com/item/1005007324368709.html) with 2.54 mm spacing, whatever the pin lenght, you will trim them anyway.
- [A GBA/GBC Game Boy serial slot](https://fr.aliexpress.com/item/1005006361884480.html). They can be cheap or not depending on the seller, do not hesitate to change.
- [A 6x6 mm pushbutton](https://fr.aliexpress.com/item/1005003938244847.html), whatever height. The softer the better if you have the choice.
- [3 x 22 µF caps with a 1206 footprint](https://fr.aliexpress.com/item/1005006022131059.html) (22 to 50 µF is OK if you have spares). Take 10V or 16V.
- [An AMS1117-3.3V converter](https://fr.aliexpress.com/item/4001104149446.html). Do not take the ADJ version proposed by default on the link page but the 3.3. The SD card is powered independently by this converter as the Pi Pico puny onboard converter is not powerfull enough.
- A [2xAA](https://fr.aliexpress.com/item/4000980371784.html) or [2xAAA](https://fr.aliexpress.com/item/1005004195965365.html) battery box with switch. Single AA battery is a bit too weak to power the device reliably (it can work or not).

**How to make it:**
- Solder the pin headers on the Pi Pico, the SD shield, the level shifter and the step-up converter. Beware of which side you solder the pin header. Check if you can drop the uf2 file to the Pico, it must blink red on and off, it's normal.
- Solder the caps and the 3.3V converter first. They are surface mount components but big enough to be soldered easily. Don't be afraid by their size. They are not tiny, you're just too far.
- Solder all parts with the minimal clearance possible against the PCB.
- Trim and reflow all pins on the back side to get a clean finish. I personally trim pins as short as possible before soldering but there is a risk of scratching the solder mask, so be carefull.
- Solder the battery box terminals and stuck the PCB on it with double sided tape for example (or hot glue if you're addicted to hot snot like me).
- You're ready to print !

The device draws about 150 mA, so couple of AAA batteries should last on average something like 5 hours of continuous printing and AA more than 20 hours. More than batteries on a modded Game Boy anyway.

**Troubleshooting**
- Last image is not written ? You've probably switched the device off while the led was still on or you forgot to tear paper with the pushbutton in tear mode.
- Last batch of images is incomplete (empty folder or just first images recorded with a multi-print) ? The batteries are completely out of juice, recharge them.
- Some image are half black/white or have missing lines / glitches when you compile the code by yourself ? See [notes](/TinyGB_Printer/Upscalerlib.h#L1) in the upscaler library, you have to manually modify some variables in PNGenc.
- Any SD card must work out of the box. If the printer is reluctant to recognize yours (it can happen, rarely), use a [low level formatting tool](https://www.sdcard.org/downloads/formatter/) or low level formatting commands like diskpart on Windows. Filesystem must be FAT32.

## Examples of use

### Making timelapse with [photo!](https://github.com/untoxa/gb-photo)
![](/Images/Showcase_2.gif)

### Saving your achievements and diploma
![](/Images/Game_examples.png)

## Compatible with all games using the Game Boy Printer
In *Italics* game working fine in automatic mode, in **bold** games requiring the pushbutton to tear paper, or tear mode. 
All known homebrews to date are compatible with the automatic mode.

(and yes, for who wonders if I'm crazy, I've tested all games in this list and save hacked most of them to access and test all cryptic printing features)

- *1942*
- *Alice in Wonderland (US version only, Euro version not printer compatible)*
- *Animal Breeder 3 (あにまるぶりーだー3)*
- *Animal Breeder 4 (あにまるぶり〜だ〜4)*
- *Aqualife (アクアライフ)*
- *Asteroids*
- *Austin Powers: Oh, Behave!*
- *Austin Powers: Welcome to My Underground Lair!*
- *Austin Power Episode 3 - Yeah Baby Yeah (leaked, rom CGBBA3P0.3)*
- *Austin Power Episode 4 - Why Make Millions (leaked, rom CGBBA4P0.0)*
- *Cardcaptor Sakura: Itsumo Sakura-chan to Issho! (カードキャプターさくら 〜いつもさくらちゃんと一緒〜)*
- *Cardcaptor Sakura: Tomoe Shōgakkō Daiundōkai (カードキャプターさくら 〜友枝小学校大運動会〜)*
- *Chee-Chai Alien (ちっちゃいエイリアン)*
- *Cross Hunter - Monster Hunter Version (クロスハンター モンスター・ハンター・バージョン)*
- *Cross Hunter - Treasure Hunter (クロスハンター トレジャー・ハンター・バージョン)*
- *Cross Hunter - X Hunter Version (クロスハンター エックス・ハンター・バージョン)*
- *Daa! Daa! Daa! Totsuzen ★ Card de Battle de Uranai de!? (だぁ!だぁ!だぁ! とつぜん★カードでバトルで占いで!?)*
- *Daikaijuu Monogatari: The Miracle of the Zone II (大貝獣物語 ザ・ミラクル オブ ザ・ゾーンII)*
- *Dejiko no Mahjong Party (でじこの麻雀パーティー)*
- *Densha de GO! 2 (電車でGO!2)*
- *Dino Breeder 3 - Gaia Fukkatsu (ディノブリーダー3 〜ガイア復活〜)*
- *Disney's Dinosaur*
- *Disney's Tarzan (ディズニーズ ターザン)*
- *Donkey Kong Country (ドンキーコング2001)*
- *Doraemon Kart 2 (ドラえもんカート2)*
- *Doraemon Memories - Nobita no Omoide Daibouken (ドラえもんメモリーズ のび太の想い出大冒険)*
- *Doraemon no Game Boy de Asobouyo Deluxe 10 (ドラえもんのGBであそぼうよ デラックス10)*
- *Doraemon no Quiz Boy (ドラえもんのクイズボーイ)*
- *Dungeon Savior (ダンジョンセイバー)*
- **E.T.: Digital Companion**
- *Fairy Kitty no Kaiun Jiten: Yousei no Kuni no Uranai Shugyou (フェアリーキティの開運辞典 妖精の国の占い修行)*
- *Fisher-Price Rescue Heroes: Fire Frenzy*
- *Game Boy Camera or Pocket Camera (ポケットカメラ)*
- *Golf Ou: The King of Golf (ゴルフ王)*
- *Hamster Club (ハムスター倶楽部)*
- *Hamster Paradise (ハムスターパラダイス)*
- *Hamster Paradise 2 (ハムスターパラダイス2)*
- *Harvest Moon 2 (牧場物語GB2)*
- *Hello Kitty no Beads Koubou (ハローキティのビーズ工房)*
- **Hello Kitty no Magical Museum (ハローキティのマジカルミュージアム)**
- *Hello Kitty Pocket Camera (ハローキティのポケットカメラ, leaked, rom GBDHKAJ0.2)*
- *Jinsei Game Tomodachi takusan Tsukurou Yo! (人生ゲーム 友達たくさんつくろうよ!)*
- *Kakurenbo Battle Monster Tactics (モンスタータクティクス)*
- *Kanji Boy (漢字BOY)*
- *Karamuchou wa Oosawagi!: Porinkiis to Okashina Nakamatachi (カラムー町は大さわぎ! 〜ポリンキーズとおかしな仲間たち〜)*
- *Karamuchou wa Oosawagi!: Okawari! (カラムー町は大さわぎ！おかわりっ！)*
- *Kaseki Sousei Reborn II: Monster Digger (化石創世リボーン2 〜モンスターティガー〜)*
- *Kettou Transformers Beast Wars - Beast Senshi Saikyou Ketteisen (決闘トランスフォーマービーストウォーズ ビースト戦士最強決定戦)*
- *Kidou Senkan Nadesico - Ruri Ruri Mahjong (機動戦艦ナデシコ ルリルリ麻雀)*
- *Kisekae Monogatari (きせかえ物語)*
- *Klax*
- *Konchuu Hakase 2 (昆虫博士2)*
- *Little Nicky*
- *Logical*
- *Love Hina Pocket (ラブ ひな)*
- *Magical Drop*
- **Mary-Kate and Ashley Pocket Planner**
- **McDonald's Monogatari : Honobono Tenchou Ikusei Game (マクドナルド物語)**
- *Mickey's Racing Adventure*
- *Mickey's Speedway USA*
- *Mission: Impossible*
- *Monster ★ Race 2 (もんすたあ★レース2)*
- *Monster ★ Race Okawari (もんすたあ★レース おかわり)*
- *Nakayoshi Cooking Series 1 - Oishii Cake-ya-san (なかよしクッキングシリーズ1 おいしいケーキ屋さん)*
- *Nakayoshi Cooking Series 2 - Oishii Panya-san (なかよしクッキングシリーズ2 おいしいパン屋さん)*
- **Nakayoshi Cooking Series 3 - Tanoshii Obentou (なかよしクッキングシリーズ3 たのしいお弁当)**
- **Nakayoshi Cooking Series 4 - Tanoshii Dessert (なかよしクッキングシリーズ4 たのしいデザート)**
- **Nakayoshi Cooking Series 5 - Cake Wo Tsukurou (なかよしクッキングシリーズ5 こむぎちゃんのケーキをつくろう!)**
- *Nakayoshi Pet Series 1: Kawaii Hamster (なかよしペットシリーズ1 かわいいハムスタ)*
- *Nakayoshi Pet Series 2: Kawaii Usagi (なかよしペットシリーズ2 かわいいウサギ)*
- *Nakayoshi Pet Series 3: Kawaii koinu (なかよしペットシリーズ3 かわいい仔犬)*
- *NFL Blitz*
- **Nintama Rantarou GB: Eawase Challenge Puzzle (忍たま乱太郎GB えあわせチャレンジパズル)**
- *Ojarumaru: Mitsunegai Jinja no Ennichi de Ojaru! (おじゃる丸 〜満願神社は縁日でおじゃる!)*
- *Pachinko Data Card - Chou Ataru-kun (Pachinko Data Card ちょ〜あたる君〜)*
- *Perfect Dark*
- *Pocket Family 2 (ポケットファミリーGB2)*
- *Pocket Kanjirou (ポケット漢字郎)*
- *Pocket Puyo Puyo-n (ぽけっとぷよぷよ〜ん)*
- *Pokémon Card GB2: Great Rocket-Dan Sanjō! (ポケモンカードGB2 GR団参上!)*
- *Pokémon Crystal (ポケットモンスター クリスタルバージョン)*
- *Pokémon Gold (ポケットモンスター 金)*
- *Pokémon Picross (ポケモンピクロス, leaked, rom DMGAKVJ0.1)*
- *Pokémon Pinball (ポケモンピンボール)*
- *Pokémon Silver (ポケットモンスター 銀)*
- *Pokémon Trading Card Game (ポケモンカードGB)*
- *Pokémon Yellow: Special Pikachu Edition (ポケットモンスター ピカチュウ)*
- *Pro Mahjong Tsuwamono GB (プロ麻雀兵 GB)*
- *Purikura Pocket 3 - Talent Debut Daisakusen (プリクラポケット3 〜タレントデビュー大作戦〜)*
- *Puzzled*
- *Quest for Camelot*
- *Roadsters Trophy*
- *Sanrio Timenet: Kako Hen (サンリオタイムネット 過去編)*
- *Sanrio Timenet: Mirai Hen (サンリオタイムネット 未来編)*
- *Shinseiki Evangelion Mahjong Hokan Keikaku (新世紀エヴァンゲリオン 麻雀補完計画)*
- *SMARTCOM (requires a hack to bypass boot sequence)*
- *Sōko-ban Densetsu: Hikari to Yami no Kuni (倉庫番伝説 光と闇の国)*
- *Super Black Bass Pocket 3 (スーパーブラックバスポケット3)*
- *Super Mario Bros. Deluxe (スーパーマリオブラザーズデラックス)*
- *Sweet Angel (スウィートアンジェ)*
- *Sylvanian Families: Otogi no Kuni no Pendant (シルバニアファミリー 〜おとぎの国のペンダント〜)*
- *Sylvanian Families 2 - Irozuku Mori no Fantasy (シルバニアファミリー2～色づく森のファンタジー)*
- *Sylvanian Families 3 - Hoshi Furu Yoru no Sunadokei (シルバニアファミリー３　星ふる夜のすなどけい)*
- *Tales of Phantasia: Nakiri's Dungeon (テイルズ オブ ファンタジア なりきりダンジョン)*
- *The Legend of Zelda: Link's Awakening DX (ゼルダの伝説 夢をみる島DX)*
- *The Little Mermaid 2: Pinball Frenzy*
- *Tony Hawk's Pro Skater 2*
- *Trade & Battle: Card Hero (トレード&バトル カードヒーロー)*
- *Tsuri Sensei 2 (釣り先生2)*
- *VS Lemmings (VS.レミングス) - Lemmings US version does not have print feature*

Want to know more about these games ? Want hints and custom saves to unlock all printing features ? Follow the [link](https://github.com/Raphael-Boichot/GameboyPrinterPaperSimulation).

## Documented limitation
- [Photo!](https://github.com/untoxa/gb-photo) standard printing (normal speed and double speed) is the only mode supported to date. Fast Printing and Transfer modes are only supported by the [Pico GB Printer](https://github.com/untoxa/pico-gb-printer), a very good dedicated printer emulator. It is also compatible with this board with minor code update (basically just change the GPIOs according to the current schematic).

## Dev notes
- The Pi Pico zero is not able to power the SD card (in writing mode) and keep track of the interrupts with the serial port at the same time, so the separate 3.3V regulator for the SD shied. The 5V step-up converter itself is rather noisy, so extra caps are necessary.
- The 5 volts line is mandatory for the level shifter so I found easier to power everything from it but it's a design choice. It also eases powering the device with a powerbank or an OTG cable from USB. In brief it is more versatile.
- The [DC-DC converter](/Images/DC_DC_characteristics.png) allows in theory using from 1 to 4 NiMH batteries. The bottleneck is when the current peaks during SD card access in write mode. If voltage drops to a critical level (about 2.7V instead of 3.3V) during this phase, the file is just discarded without warning. 2 cells is a good reliability / bulkiness compromise from my own experience but 3 to 4 would allow going farther in the discharge curve. One single cell is just not an option (it will print 10-15 images consecutively and discard all images until the next reboot, which is not very handy). The device can of course run with 2 regular alkaline batteries.
- The device does not keep track of date/time and won't, even in the future. If this is a very important feature for you, better use another device. Adding a RTC module complicates the design and the code for a very small added value. The idea here is to be (and stay) minimalistic.
- Feel free to make a lithium battery powered design of your dream. Lithium batteries are not recycled and always end as [spicy pillows](https://www.reddit.com/r/spicypillows/) so I don't like them.
- The RP2040 core for Arduino IDE takes both the Pico SDK or the Arduino commands when there is an equivalence. Here this is not rocket science either so Arduino commands are well enough.
- The inspiration came from the experience of co-developping both the hardware and software of the NeoGB Printer on ESP32: the default PCB is bulky and difficult to assemble (the random pin placement on the ESP32 dev board does not help), the device consumes a ton of current due to wifi support (which is not that reliable), the attempts to port it to a compact LilyGO board were more or less a fail because **ALL** the boards from this brand have majors issues with their charge/discharge circuit (plus they are horribly expensive for what they are: piece of crap), the dev on ESP32 was a real nightmare (you have to juggle between major hardware flaws and very questionable documentation, things that must be working and just don't work, memory management which is a mess, etc.). The Tiny GB Printer may probably be ported easily on the ESP32 zero, but it won't be by me.

## Kind warning
The code and current design come as it. If you're not happy with the current hardware, the PCB EasyEDA design or the Arduino IDE, create your own, the licence allows it ! Pull request with tested and working improvements are of course still welcomed. Feel free to design and share a 3D printed case, I won't make one.

## Aknowledgements
- [Rafael Zenaro](https://github.com/zenaro147) for the idea and because I uses chunks of code from the [NeoGB Printer project](https://github.com/zenaro147/NeoGB-Printer). This project is basically a demake and a way to get rid of the embarassing ESP32 platform.
- [Brian Khuu](https://github.com/mofosyne) for the emulator code I have butchered until it accepted to talk to my janky core 1 loop.
- [Toxa](https://github.com/untoxa) for helping me understand some PNG format subtilities that greatly improved PNGenc integration into the project.

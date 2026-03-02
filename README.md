# The TinyGB Printer - A standalone Game Boy Printer emulator with a fancy display

The TinyGB Printer is the simplest possible Game Boy printer emulator storing images directly on SD card. This is basically a demake of the ESP32 based [NeoGB printer](https://github.com/zenaro147/NeoGB-Printer) (which I co-developed with [Rafael Zenaro](https://github.com/zenaror), based on the emulator platform casted by [Brian Khuu](https://github.com/mofosyne)), but for the Raspberry Pi Pico. It stores the printed images in pixel perfect 4x PNG format. It is meant to be powered by double NiMH AA or AAA batteries, like the Game Boy Color / Pocket, so that you can recharge them with the same equipment. All parts are cheap and easy to gather online and assembly is meant to be simplistic.

**It supports all printing modes of all games compatible with the Game Boy Printer (with all possible protocol tricks like double speed, compression, custom palette, break command, arbitrary lenght printing, etc.).** So yes, all Pokémons, Link's Awakening DX, Super Mario Bros Deluxe, Game Boy Camera, all work, plus dozens of other ones. This emulator of course works with any genuine model of Game Boy (DMG, GBC, GBP, GBA). Compatibility was also confirmed with the third-party [GB Boy Colour](https://github.com/Raphael-Boichot/Knowledge-repo-about-the-Kong-Feng-GB-BOY-COLOUR) and Analogue Pocket devices.

You can copy, modify, or improve it as you wish, all sources are provided. **Just respect the license and the dedication of the authors**. In very brief, don't forget to cite Brian KHUU (emulator/parser part) and Raphaël BOICHOT (hardware/decoder part) as authors and give a link to this repository.

You can also share your images with the friendly community of Game Boy Camera enthusiasts on the [Game Boy Camera Club Telegram](https://t.me/gameboycamera). I can also help you solving issues on that Telegram.

## Make it, enjoy it !
**Nothing to compile, nothing to install, drop the UF2 file to the device and it works straight after soldering.**
![](/Images/Final_version_2.0_3.jpg)
(Credit: Raphaël BOICHOT)

## What's inside / how does it work ?
The code is basically the [Arduino Game Boy Printer emulator](https://github.com/mofosyne/arduino-gameboy-printer-emulator) (with a bit of tuning to handle the Pi Pico SDK commands) running on core 0 and a custom image decoder running on core 1 in parallel. Good timing between the two cores is critical (and required quite a lot of optimisation) as core 0 cannot miss any single interrupt while core 1 must fulfill all its tasks on time with a good safety margin. Flux control to "occupy" the Game Boy during image conversion is not necessary here.

## Easy to install
- After soldering everything, connect the Raspberry Pi Pico with a USB cable to your computer while pressing BOOT, drop the [uf2 file](/Builds) to the USB folder poping out and enjoy your device. If it makes smoke, check for shorts with a multimeter.
- **Only** if you want to modify the code and recompile it, use the [Arduino IDE](https://www.arduino.cc/en/software) equipped with the [RP2040 core for Arduino IDE](https://github.com/earlephilhower/arduino-pico). Then from the Arduino library manager install the [PNGEnc library](https://github.com/bitbank2/PNGenc), the [Bodmer TFT_eSPI libray](https://github.com/Bodmer/TFT_eSPI) and the [Adafruit Neopixel for Arduino IDE](https://github.com/adafruit/Adafruit_NeoPixel), [configure libraries](/TinyGB_Printer/TFT_setup) and choose the Raspberry Pi Pico target before compiling with default parameters (@200 MHz). I used the Arduino IDE because it gives a very user-friendly access to dev on RP2040 core without too much limitations. And the Earle Philhower RP2040 core is damn good too, it contains everything you need, tons of commented examples and ultra reactive support.

## Easy to use
- 🟢 Power the device without touching anything, the LED flashes green (display backround is green too), images are recorded and sliced automatically, this is the **automatic mode**. This is perfect for the Game Boy Camera for example. Multi-print is of course supported. It uses protocol after margins to decide wether an image is finished or not.
- 🔵 Power while pressing the pushbutton : the LED flashes blue (display backround is blue too), all images are stacked together in a single file until you press the pushbutton again to "cut paper", it's the **tear mode**. Some rare games require this (see complete game list). You can also use it freely to stack images in some other games. Just be creative !
- :red_circle: Power the device and the LED blinks red on and off in cycle: SD card not connected or not formatted in FAT32. SD card can be inserted during this step, the device will then boot normally.
- :repeat: Cycling the power switch on the battery box or using the RESET button on the board has the same effect (reset printer state and increase folder number with the regular build).

A new folder is created at each boot/RESET. Each image file has a unique ID. Flashes during printing indicate flux of data and access to SD card. Color of flashes indicates the mode (green for automatic mode, blue for tear mode). White dim LED indicates that the device is idle and ready to receive data (and that you forgot to switch it off after printing), in case you chose not using the display.

If you miss the analog feeling of thermal paper, you can still use this [thermal printer emulator](/SD/Paper_emulator) written in GNU Octave which turns your pixel perfect images into noisy paper strips with an exact aspect ratio and paper tone compared to a real Game Boy Printer equiped with genuine color / white paper. At least these will not fade with time !

**There is also an easter egg, will you find it ?** (Easy for real camera / printer nerds)

## Easy to fabricate
All the parts used here are cheap and easy to find on Aliexpress. You probably yet have some leftovers from other projects. The total price is probably well below 15€ per unit (being pessimistic), shipping of parts included, and you will have extra parts to gift some to your nerdy friends.

**Parts to order:**
- [The PCB](/PCB_2.0_with_TFT/). Order at [JLCPCB.com](https://jlcpcb.com/) (just drop the gerber .zip to the site and use default options). VAT is paid when ordering so no bad surprise for European customers. They also offer worldwide very cheap shipping options. The PCB was designed with [EasyEDA Standard Edition](https://easyeda.com/) if you want to modify something.

![](/PCB_2.0_with_TFT/PCB.png)

(note the TFT CS pin exposed, just in case, but not used with the display listed next)

The code works with the two PCB versions (1.0 without TFT and 2.0 with TFT) but I recommend ordering only the 2.0 even if you do not plan using the TFT display (just let the pins exposed or soldered with a bare female pin header).

- [A Raspberry Pi Pico Zero](https://aliexpress.com/item/1005007945758255.html). Just check pinout if seller is not the same.
- [A bare MicroSD shield](https://www.aliexpress.com/item/1005005302035188.html) without internal power converter and **without pin header** (the pins header will be mounted upside down compared to pre-soldered units so it must be soldered manually).
- [A 240x240 1.3 inches TFT Display](https://www.aliexpress.com/item/1005007143117779.html). It must be that exact one, 7 pins, without CS, 240x240 pixels, GMT130-V1.0. If you decide to use another 240x240 pixels model, CS pin is exposed and driven but it's up to you to reconfigure the TFT library if necessary.
- [A 4 gates level shifter](https://www.aliexpress.com/item/1005006255186878.html). The Pi Pico pins are not 5V rated, so the need for a level shifter. They are highly reusable in other pico related projects, so you never have too much of them in spare. If the ones you received [look like this](https://forum.arduino.cc/t/logic-level-shifter-problem/1138650?page=2), ask for refund, they are fake / defective.
- [A 5V DC-DC step-up converter](https://www.aliexpress.com/item/32809095271.html), 0.9-5V (input) to 5V (output). I know, it does not look impressive but it does perfectly the job of powering small devices. If you want the blue fullset, here another seller with a [blue PCB version](https://aliexpress.com/item/1005001640003575.html).
- [Male and female pin headers](https://www.aliexpress.com/item/4000873858801.html) with 2.54 mm spacing, whatever the pin lenght, you will trim them anyway. The female pin header is required for the TFT display only. Once again, highly reusable in other projects.
- [A GBA/GBC external link port](https://www.aliexpress.com/item/1005006358075502.html). They can be cheap or not depending on the seller, do not hesitate to change. This port is for the Gameboy Advance but it's backwards compatible (with the right cable) with any model of Game Boy.
- [A 6x6 mm pushbutton](https://www.aliexpress.com/item/1005003938244847.html), whatever height. The softer the better if you have the choice. Make sure you get the 4 pins version with the short legs.
- [2 x 22 µF caps with a 1206 footprint](https://www.aliexpress.com/item/1005006022131059.html) (22 to 50 µF is OK if you have spares). Take 10V or 16V. These come in packs of 20, you only need 2, so check your drawers just in case.
- An [AMS1117-3.3V converter](https://www.aliexpress.com/item/4001104149446.html) - Beware, take the 3.3V, not the ADJ !
- A [2xAA](https://www.aliexpress.com/item/4000980371784.html) or [2xAAA](https://www.aliexpress.com/item/1005004195965365.html) battery box **with switch** (mandatory, this is the only way to switch the device on and off). Single AA or AAA battery is too weak to power the device reliably. The choice of AA or AAA depends on which is your go to Game Boy (pocket or GBC), in order to recharge batteries by batch of 4.

Additionnaly, you would need a [GBC compatible link cable](https://www.aliexpress.com/item/1005006479007710.html). Beware, purple GBA cables (or specific GBA cables) are not compatible due to a different pinout. You can of course use genuine GBC/GBP link cables or DMG link cable with GBC/GBP adapter.

**How to make it:**
- Solder the pin headers on the Pi Pico, the SD shield, the level shifter and the step-up converter. **Beware of which side you solder the pin header**. Check if you can drop the uf2 file to the Pico, it must blink red on and off, it's normal.
- Solder the caps and the 3.3V converter first. They are surface mount components but big enough to be soldered easily. Don't be afraid by their size. They are not tiny, you're just too far.
- Solder all parts with the minimal clearance possible against the PCB.
- The TFT display can be mounted on female pin headers at this step but display is fully optional. The device works without !
- Trim and reflow all pins on the back side to get a clean finish. I personally trim pins as short as possible before soldering but there is a risk of scratching the solder mask, so be carefull.
- Clean as much as you can any flux residues with isopropanol. Flux may create parasitic noise between pins. **Beware, flushing the TFT display with isopropanol is a very reliable way of killing it !**
- Solder the battery box terminals and stuck the PCB on it with double sided tape for example (or hot glue if you're addicted to hot snot like me).
- You're ready to print !

The device can record a little more than 1 standard Game Boy Camera image for each mAh of battery storage capacity, so about 800 images with 2xAA batteries and about 2500 images with 2xAA batteries on a single charge (TFT display connected). Not connecting the TFT display would allow about 30% more images to be recorded on a single charge.

**The TinyGB Printer assembled without display (TFT display is optional) and 2xAAA battery box**
![](/Images/Final_version_2.0_4.png)
(Credit: Raphaël BOICHOT)

**The TinyGB Printer assembled with a 2xAA battery box and the TFT display compared to the 2xAAA / no TFT setup**
![](/Images/TinyGB_Printer_assembled_2.0.jpg)
(Credit: Raphaël BOICHOT)

**The TinyGB Printer with TFT display and [3D printed case](/PCB_2.0_with_TFT/3D_printed_enclosure)**
![](/PCB_2.0_with_TFT/3D_printed_enclosure/Enclosure.jpg)
(Credit: Slade1972)

**Troubleshooting**
- Last image is not written or image file is broken ? You've probably switched the device off during the PNG encoding or you forgot to tear paper with the pushbutton in tear mode.
- The TFT display does not work when you compile by yourself ? See [notes](https://github.com/Raphael-Boichot/The-TinyGB-Printer/blob/f55e4003a29216d7775f089cb41b405cfadc126f/TinyGB_Printer/TinyGB_Printer.ino#L45) for configuring the TFT_eSPI library, you have to manually edit some command for this very particular display.
- Any SD card must work out of the box, even old/slow ones. If the printer is reluctant to recognize yours, use a [low level formatting tool](https://www.sdcard.org/downloads/formatter/) or low level formatting commands like diskpart on Windows. Filesystem must be FAT32.
- The image palette is obviously wrong for a particular game ? Open an issue and I will rapidely find a solution.

## Examples of use

### Save your achievements and diploma
![](/Images/Game_examples.png)
(yes I suck at Surfing Pikachu)

### Save your preferred Game Boy Camera images
![](/Images/Game_Boy_Camera_1.png)

### Make fancy timelapses with [photo!](https://github.com/untoxa/gb-photo) (requires a [flashable camera](https://github.com/Raphael-Boichot/Game_Boy_Mini_Flashable_Camera) and a [script](/SD/Script_for_animated_gif))
![](/Images/Showcase_2.gif)

(Credit: Raphaël BOICHOT)

### Make [High Dynamic Range](/SD/Scripts_for_AEB_mode) image with [photo!](https://github.com/untoxa/gb-photo) (requires a [flashable camera](https://github.com/Raphael-Boichot/Game_Boy_Mini_Flashable_Camera))
![alt](/SD/Scripts_for_AEB_mode/Code_black_and_white/EAB_image.png)

(Credit: Raphaël BOICHOT)

### Make color images using RGB filters and [script for stacking](/SD/Scripts_for_AEB_mode) (requires a PC)
![](/SD/Scripts_for_AEB_mode/Code_color_fusion/Color_fusion.png)
![](/SD/Scripts_for_AEB_mode/Code_color_fusion/Color_fusion_08.png)

(Yes, this is done with a Game Boy Camera)

Besides scripts proposed here, you can also upload your images to the [GBCamera Android Manager](https://github.com/Mraulio/GBCamera-Android-Manager) or use the [Python HDR image processing tool](https://github.com/seb-tourneux/gbcam-hdr-utils) and apply the exact same process.

### Simulate thermal paper output with a [dedicated script](/SD/Paper_emulator)
![](/SD/Paper_emulator/Paper_out/printerPaper-dark8-0001839.png)

## Compatible with the 110 original games using the Game Boy Printer
In *Italics* games working fine in automatic mode, in **bold** games requiring the push button to tear paper, or tear mode. For who wonders if I'm crazy, after initially documenting the complete list (I hope) in sweat and blood around 2021, I've tested ALL these games and save hacked several dozens of them to access and test all cryptic printing features, took me months (most are in Japanese, many are [kusoge](https://en.wikipedia.org/wiki/Kusoge)).

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
- *Disney's Tarzan (ディズニーズ ターザン) - The game has a palette error fixed by the printer*
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
- *Hello Kitty no Magical Museum (ハローキティのマジカルミュージアム)*
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
- *Trade & Battle: Card Hero (トレード&バトル カードヒーロー) - The game has a palette error fixed by the printer*
- *Tsuri Sensei 2 (釣り先生2)*
- *VS Lemmings (VS.レミングス) - Lemmings US version does not have print feature*

Want to know more about these games ? Want hints and custom saves to unlock all printing features ? Want to read the whole story of the thermal printer emulator module ? Follow the [link](https://github.com/Raphael-Boichot/GameboyPrinterPaperSimulation).

## Also compatible with homebrews known to work with the Game Boy Printer
In *Italics* games working fine in automatic mode, **in bold** games requiring the push button to tear paper, or tear mode.

- *[2bit PXLR Studio](https://github.com/HerrZatacke/2bit-pxlr-studio) by Andreas Hahn - The first Game Boy Camera custom rom*
- *[Photo!](https://github.com/untoxa/gb-photo) by Untoxa - A Game Boy Camera custom rom, evolution of 2bit PXLR Studio*
- *[gb_snake](https://github.com/reini1305/gb_snake) by Christian Reinbacher - A one or two players version of Snake*
- **[gb_bannerprinter](https://github.com/reini1305/gb_bannerprinter) by Christian Reinbacher - A thermal paper banner printer**
- *[Blarble1290](https://8bittygames.com/blarble1290/) by Patrick Rodriguez - A text based adventure where the printer talks to you, miserable human being !*
- *A [Game Boy Printer test rom](https://github.com/mmuszkow/gbprinter) by Maciek Moszkowski - It prints text to the printer, what else ?*
- **[GBC Windows](https://archive.org/details/gbcwindows) by Rubenretro, developped with GB Studio - A spoof of Windows 3.1 with a printer function that works, mostly.**

If you find any homebrew **working for real on a genuine Game Boy Printer** (not a PC emulator please) and **not working on the TinyGB Printer**, create an issue, I will support it soon. Besides that, I'm not interested in supporting homebrews or printing modes not fully compatible with real hardware (I made an exception with Blarble1290 because it is fun to play, see the "Funfacts" section).

## Documented limitation
- [Photo!](https://github.com/untoxa/gb-photo) standard printing (normal speed and double speed) is the only mode supported to date. Fast Printing and Transfer modes are only supported by the [Pico GB Printer](https://github.com/untoxa/pico-gb-printer), a very good dedicated printer emulator. You can directly play with it by just flashing the [last release](/Builds) compiled for the current board, it's directly compatible (just drop it to the Pi-Pico and follow the instructions).
- After tens of thousands of images stored, the access to SD card can become unstable and multi-print may crash within a session. The device just reminds you that embedded systems and FAT32 formatting are not expected to manage such enormous number of file. It's probably time to move your precious files on a more durable medium anyway.

## Dev notes
- The Pi Pico zero is not able to power the SD card (in writing mode) and keep track of the interrupts with the serial port at the same time, so the separate 3.3V regulator for the SD shield. The 5V step-up converter itself is also rather noisy, so extra caps are necessary. Keeping the SD card alive in writing mode is surprisingly power hungry.
- The 5 volts line is mandatory for the level shifter so I found easier to power everything from it but it's a design choice. It also eases powering the device with a powerbank or an OTG cable from USB. In brief it is more versatile.
- The [DC-DC converter](/Images/DC_DC_characteristics.png) allows in theory using from 1 to 4 NiMH batteries. The bottleneck is when the current peaks during SD card access in write mode. If voltage drops below a critical level (about 2.7V instead of 3.3V) during this phase, even briefly, the file is just discarded without warning. 2 cells is a good reliability / bulkiness compromise from my own experience but 3 to 4 would allow going farther in the discharge curve. One single cell is just not an option (it will print 10-15 images consecutively and discard the next ones until the next reboot, which is not very handy). The device can of course run with 2 regular alkaline batteries, even pretty depleted, in case of emergency.
- The device does not keep track of date/time and won't, even in the future (same for the camera anyway, so what's the point ?). If this is a very critical feature for you, better use another device. Adding a RTC module complicates the design and the code for a very small added value. The idea here is to be (and stay) as minimalistic as possible. I've anyway added some Powershell script to do this from your preferred OS.
- Feel free to make a lithium battery powered design of your dream. Lithium batteries are not recycled, baroque in size / formats and they are prone to become [spicy pillows](https://www.reddit.com/r/spicypillows/) after some time of storage without use, whatever the conditions. I don't like them. I want my devices to be stored _easily_ without batteries if necessary. You can directly power the current board from a dedicated subboard outputting 5V via the battery pads, the 5V voltage converter can accept 5V as input without problem.
- The emulator part features no flux control (no need) apart from sending a fixed number of "busy" commands to the Game Boy (basically 1 just in case in the last release), as the image decoder running on core 1 is independent from the emulator running on core 0. The decoder must just be fast enough to be always available when called from core 0. Technically, converting a standard 160x144 pixels image from Game Boy Tile Format in memory (5760 bytes) to a compressed and indexed 640x576 pixels PNG image written on SD card takes approximately 0.5 seconds. The (near) blink of an eye compared to the real Game Boy Printer.
- I have considered the dev achieved when the device became more reliable than a BitBoy in endurance tests (because even a BitBoy can crash when abused for long in double speed mode) while having extended functions.
- The inspiration came from the experience of co-developping both the hardware and software of the NeoGB Printer on ESP32 with Rafael Zenaro. After four years tinkering with other projects in electronics and helping people to painfully compile, build and configure the NeoGB Printer, I have to admit that the system had some teething problems. The TinyGB Printer was made to be simple, reliable, no Wifi, easy to maintain.
- The device uses a quite baroque 7 pins 240x240 TFT display without CS pin. This is just because I had this one in stock as leftover from another project during dev. It is also very tiny and cute. This display is hopefully quite common on Aliexpress but on the other hand requires some annoying tuning of the TFT library. The project PCB and code can be easily modified to handle more classical TFT display.

## Funfacts
- Most games do not implement the BREAK command to abort printing but just stop transmission and relies on the very severe timing of the real printer which automatically rejects data packets with dubious dead times in between (more than 150 ms dead time between packets "kills" the whole transmission) or with incorrect checksum. The TinyGB printer detects both BREAK command and dubious packets to reject print.
- Two games have an erroneous palette with DG and LG inverted (Disney's Tarzan and Trade & Battle: Card Hero). By chance, they both use, on purpose, a weird palette not shared by any other game (maybe this explain the issue...) that can be easily detected and fixed automatically by the printer emulator. This palette issue is barely visible with an actual printer but very obvious with a printer emulator (4x pixel perfect upscaling does not lie).
- All genuine games use single packets of 640 bytes as payload for the printer (natural size to print with one pass of the printer head which is 16 pixels high) but data packets could be of any size as long as the sum of bytes transmitted is multiple of 640 in printer memory when the print command is received. A unique homebrew uses single packets of 320 bytes ([Blarble 1290](https://8bittygames.com/blarble1290/)) with print commands in between. A real printer will just refuse to print correctly single packets of 320 bytes, they have to come by two, at least. I guess that Blarble 1290 was tested on emulator only. But the game is fun. So the TinyGB printer was painfully modified to accept this particular weird protocol. Mr Rodriguez, you gave me headaches, in addition to be unreachable on the internet !
- You can still upload the original Arduino Game Boy printer emulator and it will natively work on this board with all its functions ! You will just loose the TFT display / RGB LED support as the original project does not handle the internal LED from the Pi Zero.
- Testing each major code / hardware modification requires tons of quality check in real printing conditions (Game Boy(s), serial cable, batteries, flashcart, sometimes oscilloscope, all that shit). I typically have to pass a dozen of difficult games with ruthless protocol implementation which I know will not print correctly in case of even the slightest issue with printer emulator / decoder. I sometimes try these games on other emulators / decoders for testing their accuracy. Most don't pass this crash test. The TinyGB Printer is the only emulator and decoder combo I know of that is **100% accurate**.
- A noticeable amount of compatible games listed here are considered as the [shittiest GB/GBC games](https://w.atwiki.jp/gcmatome/pages/33.html). I must admit that "playing" some of them (in one case even making a mandatory 100% to just unlock the printer menu) was less enjoyable than a visit to the dentist. I mean some games gave me nausea *systematically* when playing (yes, I'm thinking of you, E.T. The Extra-Terrestrial: Digital Companion, you piece of shit), which is *not always* the case with dental care.

## Running batteryless (at your own risk)
Foreword: SD and VCC are never connected in Game Boy genuine cables nor required by any known Nintendo product. There is no specifications for them contrary to SI and SO which are crossed. It appears that **most after market cables (at least the three I own) have SD and VCC wired AND crossed**. The device here necessitates after market cables so everything is designed considering that SD and VCC are crossed within the cable.

![](/Images/After_market_wiring.png)

Depending on the Game Boy model or flash cartridge you use, it is so possible tu run the device from the +5V (VCC) of the serial port only. For this, I recommend using the TinyGB Printer **without an SD flash cartridge** (so no EZ-FLASH Junior for example). It should put you in a situation where enough juice is available from that poor internal Game Boy power converter.

I recommend using a low forward voltage Shottky diode like a BYV1040 (like here), a 1N5817 or a BAT41 to avoid any fatal connection if the USB is connected as well as the serial. **Cathode must be mounted facing the Pi Pico**. In case of post-assembly, strictly follow the next pictures to see where to connect the +5V.

Now you're ready to run without battery and **at your own risk**. On my side, it works at least on a GBA and a GBC modded with a TFT display, genuine power converter, with different third party cables I own (the transparent green oddity sold on Amazon and the regular black crap from Aliexpress). These are the only tests I've made, you're on your own for your particular display / console combo if it does not work. If connecting the device just crashes your Game Boy, better use to the normal "with battery" setup or try another cable. You can use this [PCB](/PCB_2.0_batteryless/) to play by yourself with this design.

### Retrofit of PCB 2.0 to run batteryless (third party link cable with SD and VCC crossed required)
![](/Images/Running_batteryless.png)
![](/Images/Running_batteryless_2.jpg)
(Credit: Raphaël BOICHOT)

### Experimental PCB, untested, just try it !
![](/PCB_2.0_batteryless/PCB.png)

## Kind warning
The code and current design come as is. If you're not happy with the current hardware, the PCB EasyEDA design or the Arduino IDE, create your own, the licence allows it ! Polite pull requests with bulletproof improvements are of course always welcomed. Remind that this project is the fruit of hard work made with dedication, offered for free.

## Acknowledgements
- [Brian Khuu](https://github.com/mofosyne) for the emulator code I have butchered until it accepted to drive my janky core 1 loop. We've worked hard together to support all possible games years ago and he's the true giant whose shoulders I sit on, his emulator is incredibly reliable.
- [Slade1972](https://github.com/Slade1972) for the 3D printed enclosure (PCB 2.0 with display) and feedbacks on the building procedure and thorough compatibility tests with the Analogue Pocket.
- [Björn Heirman](https://github.com/BjornB2) for the 3D printed enclosure (PCB 1.0 without display) and feedbacks on the device ergonomy.
- [Rafael Zenaro](https://github.com/zenaror) for the initial idea ("hey man, why not doing a Pi Pico version of the NeoGB Printer now ?!") and because I reused many chunks of code from the [NeoGB Printer project](https://github.com/zenaro147/NeoGB-Printer). This project is finally more a demake than a remake of the NeoGB Printer. The TinyGB Printer was ought to be our new common project but it did not because real life is what it is.
- [Toxa](https://github.com/untoxa) for refreshing my mind about some PNG format subtelties that greatly improved PNGenc integration into the project. He is also at the origin of the batteryless design.

![Chee Chai Aliens](/Images/Chee_Chai_Aliens.gif)

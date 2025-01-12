# The TinyGB Printer - A standalone Game Boy Printer emulator with a fancy display

The TinyGB Printer is the simplest possible Game Boy printer emulator storing images directly on SD card. It is basically a demake of the ESP32 based [NeoGB printer](https://github.com/zenaro147/NeoGB-Printer) for the Raspberry Pi Pico. It stores the printed images in pixel perfect 4x PNG format. It is meant to be powered by double NiMH AA or AAA batteries, like the Game Boy Color / Pocket, so that you can recharge them with the same equipment. All parts are cheap and easy to gather online and assembly is meant to be simplistic.

**It supports all printing modes of all games compatible with the Game Boy Printer (with all possible protocol tricks like double speed, compression, custom palette, break command, arbitrary lenght printing, etc.) plus all homebrews known to date.**

You can copy, modify, sell or improve it as you want. **Just respect the license and the dedication of the authors** (in very brief, don't forget to cite Brian KHUU and Raphaël BOICHOT as authors and give a link to this repository).

## Make it, enjoy it immediately !
**Nothing to configure, nothing to install, flash the rom and it works straight after soldering.**
![](/Images/Final_version_2.0_3.jpg)
(Credit: Raphaël BOICHOT)

## What's inside / how does it work ?
The code is basically the [Arduino Game Boy Printer emulator](https://github.com/mofosyne/arduino-gameboy-printer-emulator) (with a bit of modifications to handle the RP2040 specificities) running on core 0 and a custom image decoder running on core 1 in parallel. Core 0 politely asks core 1 to convert data at certain times with a flag. Core 0 is quite busy with interrupts while core 1 is more or less idle depending on tasks asked by core 0. Good timing between the two cores is critical (and required quite a lot of optimisation) as core 0 cannot miss any interrupt and core 1 must fullfill all its tasks on time with a good safety margin.

## Easy to install
- After soldering everything, connect the Raspberry Pi Pico with a USB cable to your computer while pressing BOOT, drop the [uf2 file](/Builds) to the USB folder poping out and enjoy your device. If it makes smoke, check for shorts with a multimeter.
- If you want to modify the code and compile it, use the [Arduino IDE](https://www.arduino.cc/en/software) equipped with the [RP2040 core for Arduino IDE](https://github.com/earlephilhower/arduino-pico). Then from the Arduino library manager install the [PNGEnc library](https://github.com/bitbank2/PNGenc), the [Bodmer TFT_eSPI libray](https://github.com/Bodmer/TFT_eSPI) and the [Adafruit Neopixel for Arduino IDE](https://github.com/adafruit/Adafruit_NeoPixel), [configure libraries](/TinyGB_Printer/TFT_setup) and choose the Raspberry Pi Pico target before compiling at 250 MHz (-O3 level of compiler optimization). I used the Arduino IDE because it gives a very user-friendly access to dev on RP2040 core without too much limitations. And the Earle Philhower RP2040 core is damn good too, it contains everything you need, tons of examples and ultra reactive support.
- Want to see what's behind the scene ? Just run the device with a serial console connected (of course not mandatory, it's a standalone device), the cores are speaking to you.

## Easy to use
- 🟢 Power the device without touching anything, the LED flashes green (display backround is green too), images are recorded and sliced automatically, this is the **automatic mode**. This is perfect for the Game Boy Camera for example. Multi-print is of course supported. It uses protocol after margins to decide wether an image is finished or not.
- 🔵 Power while pressing the pushbutton : the LED flashes blue (display backround is blue too), all images are stacked together in a single file until you press the pushbutton again to "cut paper", it's the **tear mode**. Some rare games require this (see complete game list). You can also use it freely to stack images in some other games. Just be creative !
- :red_circle: Power the device and the LED blinks red on and off in cycle: SD card not connected or not formatted in FAT32. SD card can be inserted during this step, the device will then boot normally.
- :repeat: Cycling the power switch on the battery box or using the RESET button on the board has the same effect (reset printer state and increase folder number with the regular build).

A new folder is created at each boot/RESET. Each image file has a unique ID. Flashes during printing indicate flux of data and access to SD card. Color of flashes indicates the mode (green for automatic mode, blue for tear mode). White dim LED indicates that the device is idle and ready to receive data (and that you forgot to switch it off after printing), in case you chose not using the display.

There is also an easter egg, will you find it ?

## Easy to fabricate
All the parts used here are cheap and easy to find on Aliexpress. You probably yet have some leftovers from other projects. The total price is probably well below 15€ per piece, shipping of parts included, and you will have extra parts to gift some to your nerdy friends.

**Parts to order:**
- [The PCB](/PCB_2.0_with_TFT/). Order at [JLCPCB.com](https://jlcpcb.com/) (just drop the gerber .zip to the site and use default options). VAT is paid when ordering so no bad surprise for European customers. The PCB was designed with [EasyEDA Standard Edition](https://easyeda.com/) if you want to modify something.

![](/PCB_2.0_with_TFT/PCB.png)
(note the TFT CS pin exposed, just in case, but not used with the display listed next)

The code works with the two PCB versions (1.0 without TFT and 2.0 with TFT) but I recommend ordering only the 2.0 even if you do not plan using the TFT display (just let the pins exposed or soldered with a bare female pin header).

- [A Raspberry Pi Pico Zero](https://fr.aliexpress.com/item/1005005862794169.html). Just check pinout if seller is not the same.
- [A bare MicroSD shield](https://fr.aliexpress.com/item/1005005302035188.html) without internal power converter and pin header. The most simple.
- [A 240x240 1.3 inches TFT Display](https://fr.aliexpress.com/item/1005007143117779.html). It must be that exact one, 7 pins, without CS. If you decide to use another model, CS pin is exposed and driven but it's up to you to reconfigure the TFT library. 
- [A 4 gates level shifter](https://fr.aliexpress.com/item/1005004560297038.html). The Pi Pico pins are not 5V rated, so the need for a level shifter.
- [A 5V DC-DC step-up converter](https://fr.aliexpress.com/item/32809095271.html), 0.9-5V (input) to 5V (output). I know, it does not look impressive but it does prefectly the job.
- [Male and female pin headers](https://fr.aliexpress.com/item/1005007324368709.html) with 2.54 mm spacing, whatever the pin lenght, you will trim them anyway. The female pin header is required for the TFT display only.
- [A GBA/GBC Game Boy serial slot](https://fr.aliexpress.com/item/1005006361884480.html). They can be cheap or not depending on the seller, do not hesitate to change.
- [A 6x6 mm pushbutton](https://fr.aliexpress.com/item/1005003938244847.html), whatever height. The softer the better if you have the choice.
- [2 x 22 µF caps with a 1206 footprint](https://fr.aliexpress.com/item/1005006022131059.html) (22 to 50 µF is OK if you have spares). Take 10V or 16V.
- An [AMS1117-3.3V converter](https://fr.aliexpress.com/item/4001104149446.html) - Beware, take the 3.3V, not the ADJ !
- A [2xAA](https://fr.aliexpress.com/item/4000980371784.html) or [2xAAA](https://fr.aliexpress.com/item/1005004195965365.html) battery box with switch. Single AA or AAA battery is too weak to power the device reliably. The choice of AA or AAA depends on which is your go to Game Boy (pocket or GBC), in order to recharge batteries by batch of 4.

**How to make it:**
- Solder the pin headers on the Pi Pico, the SD shield, the level shifter and the step-up converter. Beware of which side you solder the pin header. Check if you can drop the uf2 file to the Pico, it must blink red on and off, it's normal.
- Solder the caps and the 3.3V converter first. They are surface mount components but big enough to be soldered easily. Don't be afraid by their size. They are not tiny, you're just too far.
- Solder all parts with the minimal clearance possible against the PCB.
- The TFT display can be mounted on female pin headers at this step but display is fully optional. The device works without !
- Trim and reflow all pins on the back side to get a clean finish. I personally trim pins as short as possible before soldering but there is a risk of scratching the solder mask, so be carefull.
- Clean any flux residues with isopropanol. Flux may create parasitic noise between pins.
- Solder the battery box terminals and stuck the PCB on it with double sided tape for example (or hot glue if you're addicted to hot snot like me).
- You're ready to print !

The device can record a little more than 1 standard Game Boy Camera image for each mAh of battery storage capacity, so about 800 images with 2xAA batteries and about 2500 images with 2xAA batteries on a single charge (TFT display connected). Not connecting the TFT display would allow about 30% more images to be recorded on a single charge.

**The TinyGB Printer assembled without display (TFT display is optional) and 2xAAA battery box**
![](/Images/Final_version_2.0_4.png)
(Credit: Raphaël BOICHOT)

**The TinyGB Printer assembled with a 2xAA battery box and the TFT display compared to the 2xAAA / no TFT setup**
![](/Images/TinyGB_Printer_assembled_2.0.jpg)
(Credit: Raphaël BOICHOT)

**Troubleshooting**
- Last image is not written ? You've probably switched the device off while the led was still on or you forgot to tear paper with the pushbutton in tear mode.
- Last batch of images is incomplete (empty folder or just first images recorded with a multi-print) ? The batteries are completely out of juice, recharge them.
- Some image are half black/white or have missing lines / glitches when you compile the code by yourself ? See [notes](/TinyGB_Printer/Upscalerlib.h#L1) in the upscaler library, you have to manually modify some variables in PNGenc.
- Any SD card must work out of the box. If the printer is reluctant to recognize yours (it can happen, rarely), use a [low level formatting tool](https://www.sdcard.org/downloads/formatter/) or low level formatting commands like diskpart on Windows. Filesystem must be FAT32.
- The image palette is obviously wrong for a particular game ? Open an issue and I will rapidely find a solution.

## Examples of use

### Making timelapse with [photo!](https://github.com/untoxa/gb-photo)
![](/Images/Showcase_2.gif)

(Credit: Raphaël BOICHOT)

### Saving your achievements and diploma
![](/Images/Game_examples.png)
(yes I suck at Surfing Pikachu)

## Compatible with ALL games using the Game Boy Printer
In *Italics* games working fine in automatic mode, in **bold** games requiring the push button to tear paper, or tear mode. 
All known homebrews to date are compatible with the automatic mode.

(and yes, for who wonders if I'm crazy, I've tested ALL these 110 games and save hacked several dozens of them to access and test all cryptic printing features, took me months)

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

Want to know more about these games ? Want hints and custom saves to unlock all printing features ? Follow the [link](https://github.com/Raphael-Boichot/GameboyPrinterPaperSimulation).

## Documented limitation
- [Photo!](https://github.com/untoxa/gb-photo) standard printing (normal speed and double speed) is the only mode supported to date. Fast Printing and Transfer modes are only supported by the [Pico GB Printer](https://github.com/untoxa/pico-gb-printer), a very good dedicated printer emulator. It must natively be compatible with this board after minor code update (basically just change the GPIOs according to the current schematic).
- After tens of thousands of images stored, the access to SD card can become unstable and multi-print may crash within a session. The device just reminds you that embedded systems are not expected to manage such enormous filesystem. It's probably time to move your precious files on a more durable medium anyway.

## Dev notes
- The Pi Pico zero is not able to power the SD card (in writing mode) and keep track of the interrupts with the serial port at the same time, so the separate 3.3V regulator for the SD shied. The 5V step-up converter itself is also rather noisy, so extra caps are necessary.
- The 5 volts line is mandatory for the level shifter so I found easier to power everything from it but it's a design choice. It also eases powering the device with a powerbank or an OTG cable from USB. In brief it is more versatile.
- The [DC-DC converter](/Images/DC_DC_characteristics.png) allows in theory using from 1 to 4 NiMH batteries. The bottleneck is when the current peaks during SD card access in write mode. If voltage drops below a critical level (about 2.7V instead of 3.3V) during this phase, even briefly, the file is just discarded without warning. 2 cells is a good reliability / bulkiness compromise from my own experience but 3 to 4 would allow going farther in the discharge curve. One single cell is just not an option (it will print 10-15 images consecutively and discard the next ones until the next reboot, which is not very handy). The device can of course run with 2 regular alkaline batteries, even pretty depleted, in case of emergency.
- The device does not keep track of date/time and won't, even in the future (same for the camera anyway, so what's the point ?). If this is a very important feature for you, better use another device. Adding a RTC module complicates the design and the code for a very small added value. The idea here is to be (and stay) minimalistic. I've anyway added some script to do this from your OS.
- The Pi Pico requires overclocking for the moment (which increases current draw) but I'm pretty sure it would run at a 125 or 133 MHz with some code optimization. The fact is that the PNG encoder running on core 1 sometimes affect the stability of serial protocol handled by core 0 at standard running frequency, which is not supposed to happen. I continue working loosely on this "issue" at my own pace, as the device is currently perfectly stable at 250 MHz.
- Feel free to make a lithium battery powered design of your dream. Lithium batteries are not recycled and always end as [spicy pillows](https://www.reddit.com/r/spicypillows/) so I don't like them. You can directly power the current board from a dedicated subboard outputting 5V via the battery pads, the 5V voltage converter can accept 5V as input without problem.
- The RP2040 core for Arduino IDE takes both the Pico SDK or the Arduino commands when there is an equivalence. Here this is not rocket science either so Arduino commands are well enough for the task.
- The emulator part features no flux control apart from sending a fixed number of "busy" commands to the Game Boy as the image decoder running on core 1 is independant from the emulator running on core 0. The decoder must just be fast enough to be always available when called from core 0. Technically, converting a standard 160x144 image from Game Boy Tile Format in memory (640x9 bytes) to a 4x PNG image written on SD card take approximately 0.5 seconds. The blink of an eye.
- I have considered the dev achieved when the device became more reliable than a BitBoy in endurance tests (because even a BitBoy can crash when abused for long in double speed mode) while having extended functions.
- The inspiration came from the experience of co-developping both the hardware and software of the NeoGB Printer on ESP32: the default PCB is bulky and difficult to assemble (the random pin placement on the ESP32 dev board does not help), the device consumes a ton of current due to wifi support (which is even not reliable), the attempts to port it to a compact LilyGO board were more or less a fail because **ALL** the boards from this brand have majors issues with their charge/discharge circuit (plus they are horribly expensive for what they finally are: piece of crap), the dev on ESP32 was a real nightmare (you have to juggle between major hardware flaws and very questionable documentation, things that must be working and just don't work, memory management which is a mess, total unstability at 240 MHz, etc.). The Tiny GB Printer may probably be ported easily on the ESP32 zero, but it won't be by me.
- Testing each major code / hardware modification requires tons of quality check in real printing conditions (Game Boy, serial cable, batteries, flashcart, sometimes oscilloscope, all that shit). I typically have to pass a dozen of difficult games with ruthless protocol implementation which I know will not print correctly in case of even the slightest issue with printer emulator. I use these games too to assess other emulators / decoders accuracy for fun (most don't pass).

## Funfacts
- Most games do not implement the BREAK command to abort printing but just stop transmission and relies on the very severe timing of the printer which automatically rejects data with dubious dead times. The TinyGB printer detects both BREAK command and dubious packets to reject print.
- Two games have an erroneous palette with DG and LG inverted (Disney's Tarzan and Trade & Battle: Card Hero). By chance, they both use on purpose a weird palette not shared by any other game that can be easily detected and fixed automatically by the printer emulator. This palette issue is barely visible with an actual printer but very obvious with a printer emulator (4x pixel perfect upscaling does not lie).
- All genuine games uses packets of 640 bytes as payload to the printer (natural size to print with one pass of the printer head which is 16 pixels high) but in theory it could be different. A unique homebrew uses packets of 320 bytes ([Blarble 1290](https://8bittygames.com/blarble1290/)). My own tests show that a real printer (Hosiden series) refuses to print with this game. I guess that it was tested on emulator or on a Seiko series only. The TinyGB printer was painfully modified to accept this particular weird protocol, I do not thank you Mr Rodriguez.
- You can still upload the original Arduino Game Boy printer emulator and it will natively work on this board with all its functions ! You will just loose the external LED support as the original project does not handle the internal RGB LED from the Pi Zero.

## Kind warning
The code and current design come as it. If you're not happy with the current hardware, the PCB EasyEDA design or the Arduino IDE, create your own, the licence allows it ! Polite pull requests with tested and working improvements are of course still welcomed. Remind that this is a hard work made with dedication, for FREE.

## Aknowledgements
- [Björn Heirman](https://github.com/BjornB2) for the 3D printed enclosure and feedbacks on the device ergonomy.
- [Rafael Zenaro](https://github.com/zenaro147) for the idea and because I uses chunks of code from the [NeoGB Printer project](https://github.com/zenaro147/NeoGB-Printer). This project is basically a demake and a way to get rid of the embarassing ESP32 platform.
- [Brian Khuu](https://github.com/mofosyne) for the emulator code I have butchered until it accepted to talk to my janky core 1 loop.
- [Toxa](https://github.com/untoxa) for refreshing my mind about some PNG format subtilities that greatly improved PNGenc integration into the project.

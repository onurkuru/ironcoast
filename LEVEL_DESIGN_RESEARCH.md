# Metal Slug bölüm araştırması ve Iron Coast harita revizyonu

8 Eylül 2026 · İncelenen Iron Coast sürümü: `dfc8fe7` / 0.2.2

**Durum:** Araştırma ve uygulanacak bölüm tasarımı. Bu belge yeni oynanabilir haritaların uygulandığı anlamına gelmez.

## Sonuç

Iron Coast'un problemi yalnızca arka plan görselinin boş olması değil. Mimari, üzerinde oynanan geometriyi ve çatışmaları belirlemiyor. Metal Slug'dan çıkarılacak temel ilke: önce sokak, depo, köprü, vagon veya fabrika gibi bir mekân kurmak; sonra yürünecek yüzeyleri, düşman mevzilerini ve girişlerini o mekâna yerleştirmek.

Metal Slug'ın her bölümü dikey değildir. İlk oyunun şehir bölümündeki uzun, düz yol bile cephelerin kalınlığı, balkonlar, ara sokaklar, araçlar ve farklı yüksekliklerdeki tehditlerle doludur. Sadece zemine delik açmak veya arkaya bir şehir resmi koymak aynı sonucu vermiyor.

## Kapsam ve kanıt sınırı

Metal Slug 1'in altı görevinin akışı; Metal Slug 3'ün beş görevinin ana rotaları ve alternatif geçişleri incelendi. Araştırma bütün ana serinin bütün sürümlerini kapsamaz.

Kaynaklar: bölüm görüntülerinin doğrudan görsel incelemesi, oyuncuların birinci elden görev rehberleri, nesne davranışı araştırması ve SNK'nin resmi tarihçesi. Birleştirilmiş harita görselleri çarpışma verisi değildir; hareketli düşmanların bütün tetikleyicilerini göstermez. Bu nedenle aşağıda **özgün oyunlara ait kesin piksel koordinatı veya eksiksiz düşman sayısı iddiası yoktur**. Konumlar, oynanıştaki ilişkileriyle tarif edilir. Iron Coast için verilen koordinatlar yeni tasarım önerisidir.

VGMaps'teki ilk oyun setinde Mission 4 yok; Mission 3 ve Final Mission görselleri görevin tamamını kapsamıyor. Bu kısımlar görev rehberiyle tamamlandı. Metal Slug 3'ün finali iki ayrı harita görselinde incelendi. [Harita dizini](https://vgmaps.com/Atlas/Neo-Geo/index.htm), [ilk oyun görev rehberi](https://gamefaqs.gamespot.com/ps/573212-metal-slug/faqs/7), [MS3 görev rehberi](https://gamefaqs.gamespot.com/arcade/577440-metal-slug-3/faqs/38376).

## 1. Metal Slug 1: altı görevin mekânsal düzeni

Aşağıdaki mimari değerlendirmeler, bağlantılı harita görsellerinin doğrudan gözlemidir. Düşman düzeni özeti sonraki tabloda ayrı verilir.

| Görev | Görseldeki platform ve mekân yapısı | Iron Coast'a uygulanacak ders |
|---|---|---|
| 1 | Taş kalıntılar, enkaz, sığ su, kazıklı ahşap yapılar; son kısımda kayalarla yükselen şelale yaklaşımı. | Zemin kotu değişiklikleri yapı ve araziyle açıklanmalı. Baraka tabanı, balkon ve kaya yüzeyi birbirinden farklı okunmalı. |
| 2 | Yıkılmış demiryolu, su üstündeki köprü kalıntıları, uzun ahşap eğimler ve yükselen ray güzergâhı. | Boşluk köprünün hasarıdır; rastgele ceza değildir. Yolun taşıyıcı kolonları ve altındaki su görünür olmalı. |
| 3 | Görselin kapsadığı bölümde birbirine bakan kaya çıkıntılarıyla gerçek dikey tırmanış; ahşap bağlantılar ve üstte karla kaplı üs. | Dikey bölüm, yatay yola birkaç levha eklemekten farklıdır; kamera ve karşılıklı çıkıntılar birlikte tasarlanır. |
| 4 | Rehberde bar içi/üst kat, tepeler ve mağara geçişleri anlatılır; bu araştırmada tam harita görseli bulunamadı. | İç mekân ile dış arazi farklı karşılaşma alanları olmalı. Bu satır görsel harita ölçümüne dayanmıyor. |
| 5 | Uzun taş sokak; cepheler, derin pencere boşlukları, balkonlar, arkayı gösteren ara sokaklar, araçlar ve iki katlı geçit. | Kullanıcının istediği bina derinliği için en doğrudan referans. Düz ana yol korunurken dolu ve katmanlı bir mekân kurulabilir. |
| Final | Mevcut görselde orman yaklaşımı, asma köprü ve rampayla bağlanan iki seviyeli büyük köprü. | Üstteki bir kopukluk, alttaki geçişle rota seçimine dönüşebilir. Görsel final çatışmasının tamamını göstermiyor. |

Görsel kaynaklar: [Mission 1](https://vgmaps.com/Atlas/Neo-Geo/MetalSlug-Mission1.png), [Mission 2](https://vgmaps.com/Atlas/Neo-Geo/MetalSlug-Mission2.png), [Mission 3](https://vgmaps.com/Atlas/Neo-Geo/MetalSlug-Mission3.png), [Mission 5](https://vgmaps.com/Atlas/Neo-Geo/MetalSlug-Mission5.png), [Final'in köprü kısmı](https://vgmaps.com/Atlas/Neo-Geo/MetalSlug-FinalMission.png).

### Düşman konumları ve karşılaşma sırası

| Görev | Yerleşim ve giriş biçimi |
|---|---|
| 1 | Suda dalgıçlar; kulübelerin üstünde bombacılar; yükselen yaklaşımda tanklar. Cephe ve üst mevzi birlikte baskı yaratıyor. |
| 2 | Yukarıdan paraşütçüler, aşağıdaki teknelerden saldırılar, rampanın tepesinde ve kenarında askerler. |
| 3 | Uzun çıkıntıda kalkanlı asker ve roketçi; üst kotlardan saldırı; tırmanış sonunda Allen O'Neil karşılaşması. |
| 4 | Balkon roketçisi, tepelerdeki çukurlarda bombacılar, yol üzerindeki tanklar; yükseklik siper görevi görüyor. |
| 5 | Pencere mevzileri, asker üreten araçlar, sokak tankları; iki katlı geçitte farklı seviyelerden motorlu saldırılar. |
| Final | Köprüde üst ve alt tehditler; tekne üzerinde hava saldırısı; kıyıya çıkıştan sonra kara çatışması. |

Kaynak: [achtungnight'ın birinci elden görev rehberi](https://gamefaqs.gamespot.com/ps/573212-metal-slug/faqs/7). Özet, zorluk seviyelerine göre bütün spawn adetlerini temsil etmez.

Pencereye yerleşen bombacı ve halata bağlı asker gibi davranışların ayrı nesne türleri olarak bulunması da önemli: konum, yalnızca aynı askerin `y` değerini değiştirmekten ibaret değil. [Nesne araştırması](https://randomhoohaas.flyingomelette.com/msmia/1/ob.html). Nesne kataloğundaki her kaydın her görevde kullanıldığı varsayılmadı.

## 2. Metal Slug 3: görevler ve rota yapısı

SNK, MS3'ün seçime göre dallanan haritasını özellikle vurguluyor. Dolayısıyla referansın önemli bir parçası, dekor yoğunluğu kadar aynı hedefe farklı mekânlardan ulaşmak. [SNK resmi tarihçe](https://www.snk-corp.co.jp/us/anniversary/metalslug30th/history/).

| Görev | İncelenen rota yapısı | Platform ve derinlik gözlemi |
|---|---|---|
| 1 | Kıyıdan yüzey/tekne, sualtı veya kanal geçişleri; ortak boss yaklaşımı. | Yatay sahile ek olarak aşağı inip tekrar çıkan sualtı kolu; giriş ve birleşmeler fiziksel geçişlerle okunuyor. |
| 2 | Ana dağ güzergâhı ve geri bağlanan buz mağarası. | Eğimli arazi ile iki seviyeli mağara farklı hareket ritimleri oluşturuyor. |
| 3 | Sualtı girişlerinden ayrılan kollar ve yüzey üs yolu; fabrika ve ortak boss. | Borular, kutular, merdivenler, taşıyıcı kirişler ve çok katlı fabrika şaftı. Üst kat gerçekten ulaşılacak bir yer. |
| 4 | Üstte piramit güzergâhı; altta üs ve yeraltı kolları. Mumya, böcek tüneli ve mahzen geçişleri. | Büyük yapı silueti rota seçimini açıklıyor; kapaklar ve şaftlar alanları bağlıyor. |
| Final | Hava yaklaşımı → üs → uzay yolculuğu → ana gemi → kaçış/final. | Tek bir uzatılmış yatay platform yerine, farklı hareket biçimlerine sahip bölüm parçaları var. |

Doğrudan incelenen haritalar: [MS3 Mission 1](https://vgmaps.com/Atlas/Neo-Geo/MetalSlug3-Mission1.png), [Mission 2](https://vgmaps.com/Atlas/Neo-Geo/MetalSlug3-Mission2.png), [Mission 3](https://vgmaps.com/Atlas/Neo-Geo/MetalSlug3-Mission3.png), [Mission 4](https://vgmaps.com/Atlas/Neo-Geo/MetalSlug3-Mission4.png), [Final 1](https://vgmaps.com/Atlas/Neo-Geo/MetalSlug3-FinalMission(Part1).png), [Final 2](https://vgmaps.com/Atlas/Neo-Geo/MetalSlug3-FinalMission(Part2).png). Tablo ana bağlantıları özetler; her oda ve kapının teknik dökümü değildir.

Düşman düzeninde kıyıdaki kara/hava karışımı, mağaradaki üst-alt mevziler, fabrikadaki çok katlı muhafızlar, dar tünellerde tavan/zemin tehditleri ve final koridorlarında arkadan ilerleyen baskı öne çıkıyor. Rota seçimi karşılaşılacak düşman grubunu da değiştiriyor. [MS3 görev rehberi](https://gamefaqs.gamespot.com/arcade/577440-metal-slug-3/faqs/38376).

## 3. Mevcut Iron Coast verisinin denetimi

`data/campaign.json` doğrudan sayıldı. Ana zemin, bütün görevlerde `y=232`. Boşluk genişlikleri 48–64 piksel. Üst yüzey sayısı otomatik eklenen ara basamakları da içeriyor; tek başına bu sayı her platformun hatalı olduğunu göstermez.

| Harita | Genişlik | Zemin boşluğu | Üst yüzey | Ana zemindeki düşman | Üstteki kara düşmanı | Drone |
|---|---:|---:|---:|---:|---:|---:|
| RUSTED HARBOR | 3600 | 3 | 10 | 13 | 3 | 2 |
| TOXIC MARSH | 3840 | 5 | 10 | 12 | 4 | 5 |
| IRONLINE | 4000 | 6 | 10 | 13 | 3 | 3 |
| EMBER FOUNDRY | 3680 | 3 | 10 | 11 | 3 | 3 |
| STORM RELAY | 3840 | 4 | 13 | 10 | 4 | 5 |
| FINAL WAVE | 4200 | 4 | 11 | 13 | 4 | 4 |
| **Toplam** | **23160** | **25** | **64** | **72** | **21** | **22** |

115 başlangıç düşmanının 93'ü kara birimi; bunların yaklaşık %77'si aynı ana zemin yüksekliğinde başlıyor. Boss ve sonradan üretilen birimler bu sayıya dahil değil.

Kodda görülen nedenler:

- `tools/create_campaign.py`: üst mevzilerin bir bölümü platform sırasının üçe bölünmesine göre ekleniyor. Kapı, pencere veya çatışma amacı tanımlanmıyor.
- Aynı üreticide varil ve kasalar görevler boyunca tekrar eden x konumlarına dağıtılıyor. Kontrol noktaları da boşluktan sonraki zemin parçalarından türetiliyor.
- `src/game.h`: platform yalnızca dikdörtgen ve `oneWay`; bina, çatı, pencere, giriş kapısı ve dalga tetikleyicisi yok.
- `src/game.cpp`: düşmanlar bölüm başında kuruluyor, kameraya yaklaşınca etkinleşiyor. Kapının açılması ve askerin dışarı çıkması gibi görünür bir giriş zinciri yok. Kamera yalnız yatay takip ediyor.
- `src/render.cpp`: yakın oynanış alanında binalar yerine tekrar eden platform şeritleri var. Uzak panorama içindeki binaların çatısına çıkılamıyor; cephe, geçit ve çarpışma birbirine bağlı değil.

**Teşhis:** Daha fazla parallax veya ışık, bu geometri ve karşılaşma eksikliğini tek başına gidermez. SDL kullanılması da bu eksikliğin nedeni değil; değişmesi gereken bölüm verisi, sahne düzeni ve davranış sistemi.

## 4. Altı görev için yeni mekânsal tasarım

Aşağıdakiler özgün Iron Coast önerileridir; Metal Slug haritalarının kopyası değildir.

| Görev | Yeni mekân zinciri | Oynanabilir mimari ve düşman düzeni |
|---|---|---|
| RUSTED HARBOR | Liman sokağı → depo → konteyner avlusu → kanal köprüsü → vinç sahası | Balkon ve depo çatısı alternatif üst yol. Kapıdan piyade, üst pencerede bombacı, avluda kalkanlı birlik. Üç mevcut boşluk kaldırılır; kanal kesintisiz köprüyle geçilir. |
| TOXIC MARSH | Pompa yerleşkesi → kazıklı evler → arıtma havuzu → pompa istasyonu | Zeminsiz levhalar yerine ayaklı iskele, boru köprüsü ve bakım balkonu. Suya düşüş geri çıkılabilen alt servis yoluna bağlanır. Drone çıkışı pompa bacasında görünür. |
| IRONLINE | Yükleme istasyonu → yük vagonları → yolcu/cephane vagonu → lokomotif | Vagon içi ve tavanı iki rota. Bağlantılarda kör boşluk yerine kuplör/servis basamağı. Düşman kapıdan çıkar veya komşu vagon üzerinden gelir. |
| EMBER FOUNDRY | Fabrika kapısı → üretim salonu → döküm hattı → fırın odası | Kolonla taşınan asma katlar, pres odası, bakım merdivenleri. Üst bombacı ve alt zırhlı birlik karşılıklı baskı kurar. Erimiş metal kanalı gerçek, işaretli bir tehlikedir. |
| STORM RELAY | İdari bina → bakım katları → bağlantı köprüsü → anten çatısı | Gerçek yükseliş; dikey kamera ve asansör gerektirir. Pencerelerden mevziler, kapılardan takviye. Çatı silueti yaklaşan hedefi gösterir. |
| FINAL WAVE | İskele → hangar → komuta binası → tahliye güvertesi | Hangar alt yolu ile üst bakım yolu birleşir. Önceden görülen kapı ve pencerelerden kademeli saldırı. Son arena kaçınma alanı açık, zemini okunur bir güverte olur. |

### RUSTED HARBOR: uygulanacak ilk blok planı

Koordinatlar mevcut 3600 px dünya genişliğine göre önerildi. `y` aşağı doğru büyür; ayak hizası yüzeyin üst kenarıdır. Mevcut sıçrama yaklaşık 51 px yükseldiği için ara basamaklar 40 px kot farkıyla tasarlanır. Geometrik mesafe tek başına erişilebilirlik kanıtı değildir; uygulamada karakter genişliği ve hareketli test de gerekir.

| Dünya aralığı | Yapı / yüzey | Düşman yerleşimi ve tetikleyici |
|---|---|---|
| 0–480 | Kesintisiz liman sokağı. 270–355 arası yükleme basamağı `y192`; 355–475 balkon `y152`. | Sokakta tek muhafızla tanıtım. İleride görünen giriş kapısı ilk takviyenin kaynağı; arkada aniden belirme yok. |
| 480–1120 | Depoda alt koridor `y232`; 560–1050 çatı yolu `y152`. İki uçta `y192` basamakları. | Alt kapıdan iki piyade; üst pencereden hazırlığı görünen bombacı. Aynı anda değil, oyuncunun girişine göre sıralanır. |
| 1120–1680 | Avluda 1200–1330 konteyner `y192`, 1330–1450 konteyner `y152`, 1450–1535 basamak `y192`. | Avlunun çıkışında kalkanlı birlik; konteyner üstü açı değiştirme fırsatı. Araç ve yakın dövüş yolu açık bırakılır. |
| 1680–2280 | Kanal üstünde kesintisiz köprü `y232`; ayakları ve su yüzeyi görünür. Opsiyonel servis çıkıntısı. | Karşı köprü başında sabit mevzi; hava takviyesi önce uzakta görünür. İlk bölümde ölüm çukuru yok. |
| 2280–3180 | Sevkiyat binası, yükleme rampası ve açık vinç avlusu. Araç için geniş alt yol. | Binanın sürgülü kapısı açılır, sınırlı takviye gelir. Üst mevzi önce tek başına tanıtılır, sonra yer birimi eklenir. |
| 3180–3600 | Vinç temeli üzerinde kesintisiz boss arenası. Arka hangar ve raylar yapının ölçeğini açıklar. | Boss giriş/kamera kilidi arena sınırına bağlıdır. Geri çekilme alanı dekor veya rastgele çukurla daraltılmaz. |

Kapı dalgası için başlangıç hedefi: yaklaşık 0,6 saniye görünür hazırlık, ardından asker çıkış animasyonu; oyuncuyla aynı noktada spawn olmama ve aktif tehdit sayısını sınırlandırma. Bunlar özgün dengeleme değerleridir, Metal Slug'dan ölçülmüş süreler değildir.

## 5. Sanat, ışık ve çarpışma birlikte değişmeli

Her yapı paketi en az cephe, yan yüz, çatı kenarı, kapı, pencere, taşıyıcı ve taban birleşimi içermeli. İlk paketler: liman deposu, işçi binası, konteyner, köprü ayağı ve vinç hangarı. Küçük Türk öğeleri tabela veya mimari ayrıntı olabilir; görev metni ve arayüz İngilizce kalır.

Katman sırası: uzak kent → orta mesafe yapı → oynanış cephesi → aktörler → seyrek yakın kenar öğeleri. Oynanış cephesi ve ona bağlı çatı aynı dünya hareketini paylaşmalı; platform farklı parallax katsayısıyla kaymamalı. Yan sokağın içi karanlık, yakın cephe kenarı daha belirgin olabilir. Zeminin görünür üst kenarı ile çarpışma yüzeyi tek veri kaynağından çıkmalı.

Işıklar pencere, kapı, sokak lambası, araç ve makineye bağlanmalı. Bina önünde ışık lekesi varsa kaynağı anlaşılmalı. Ayak temasını örten sis ve parlaklık azaltılmalı; destek gölgesi bina ile zemin birleşimini anlatmalı. REPLACED yönündeki atmosfer, bu fiziksel sahnenin üzerine kurulmalı. Bu belgede REPLACED için yeni teknik inceleme yapıldığı iddia edilmiyor.

## 6. Uygulama sırası ve kabul ölçütleri

1. **İlk liman bloğu:** yapı/yüzey kimlikleri, görünür depo ve balkon, kesintisiz sokak, elle tasarlanmış düşman konumları. Önce yürünür bir mimari dil doğrulanır.
2. **Karşılaşmalar:** kapı/pencere girişleri, uyarı animasyonu, sınırlı dalgalar; kameraya yaklaşınca etkinleşen mevcut sistemden sahneye bağlı tetikleyicilere geçiş.
3. **Bütün liman:** iki rotanın birleşmeleri, köprü, araç geçişi, boss arenası. Sonra diğer beş görev aynı şemayla ayrı ayrı yazılır; tek desen çoğaltılmaz.
4. **Dikey sistemler:** dikey kamera, asansör ve gerekiyorsa merdiven. Bunlar mevcut motorda hazır değil; yalnız harita JSON'u değiştirerek tamamlanamaz.

Kontroller:

- Ana yol ve her ödül yolu hareket simülasyonuyla baştan sona ulaşılabilir olmalı; bütün çatı çıkışları geri dönüşe izin vermeli.
- Karakter, düşman ve araç ayakları görünür yüzeye oturmalı. Kamera hareketinde cephe, collider ve ışık kaynağı birbirinden ayrılmamalı.
- Balkon altından geçerken karakter başı görünmez duvara takılmamalı; çatıdan ateş eden düşmanın mermisi kendi cephe dekoruna çarpmamalı.
- Takviye aynı tetikleyicide sınırsız tekrar etmemeli; checkpoint dönüşü karşılaşmayı tutarlı yeniden kurmalı.
- Her çukur veya düşme alanının mekânsal gerekçesi, önceden görünür sınırı ve tanımlı sonucu bulunmalı. İlk liman görevinde zorunlu ölüm çukuru olmamalı.
- macOS'ta hareketli oynanışla doğrulama yapılmalı. Vita için derleme ayrı, fiziksel cihaz performans testi ayrı raporlanmalı.

Bu revizyonda oyun kodu ve dağıtım paketleri değiştirilmedi. Araştırma, mevcut 0.2.2 haritalarının denetimi ve altı görev için uygulanabilir tasarım tamamlandı.

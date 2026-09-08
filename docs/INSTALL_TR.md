# Iron Coast — PS Vita kurulum rehberi

[VPK indir](https://github.com/onurkuru/ironcoast/releases/latest) · [Ana sayfa](../README.md)

## Gerekenler

Homebrew çalıştırmaya hazır PS Vita, çalışan [VitaShell](https://github.com/TheOfficialFloW/VitaShell) ve `ux0:` üzerinde en az 100 MB boş alan. Bu alan, kurulum sırasında VPK ile açılmış dosyaların birlikte bulunması için önerilen paydır. Oyun için ayrıca ROM, veri paketi veya özel eklenti gerekmiyor.

Bu rehber cihazın homebrew kurulumunun hazır olduğunu varsayar. Hazır değilse güncel [Vita Hacks Guide](https://vita.hacks.guide/) üzerinden cihaz kurulumunu tamamlayın.

## 1. Doğru dosyayı indirin

GitHub **Releases → Assets** bölümünden `Iron-Coast-PSVita-v0.2.5.vpk` dosyasını indirin. “Source code” ZIP/TAR dosyaları oyunun kurulum paketi değildir.

İsterseniz `SHA256SUMS.txt` dosyasını da indirip VPK'nın SHA-256 değerini karşılaştırın. Windows PowerShell'de:

```powershell
Get-FileHash .\Iron-Coast-PSVita-v0.2.5.vpk -Algorithm SHA256
```

Linux'ta:

```sh
sha256sum -c SHA256SUMS.txt
```

## 2. USB ile aktarın

1. VitaShell'i açın. **START** ile ayarlara girin.
2. **SELECT button** seçeneğini **USB** yapın. **USB device** altında, cihazınızda `ux0:` olarak kullanılan depolamayı seçin.
3. Veri aktarabilen USB kablosunu bağlayın. VitaShell'de **SELECT** tuşuna basın.
4. Bilgisayarda açılan depolamada `VPK` klasörü oluşturun; indirdiğiniz `.vpk` dosyasını buraya kopyalayın. Vita'daki yol `ux0:VPK/` olacaktır.
5. Kopyalama bitince depolamayı bilgisayardan güvenle çıkarın, ardından VitaShell'de USB modundan çıkın.

**FTP seçeneği:** Vita ve bilgisayar aynı yerel ağdayken VitaShell ayarındaki SELECT işlevini **FTP** seçin. SELECT'e basıp cihazın gösterdiği adres ve portu FTP istemcinize yazın. Dosyayı `ux0:VPK/` klasörüne gönderin. Aktarım tamamlanınca bağlantıyı kapatın.

## 3. Kurun ve açın

1. VitaShell'de `ux0:VPK/` klasörüne girin.
2. `Iron-Coast-PSVita-v0.2.5.vpk` dosyasını seçip onay tuşuna basın; kurulum sorusunu onaylayın. VitaShell'de onay genellikle **Cross**, bazı ayarlarda **Circle** olabilir.
3. Kurulum bitince LiveArea'ya dönün.
4. Turuncu vinç kıskacı ve turkuaz çekirdek ikonlu **Iron Coast: Scrap Tide** balonunu açın; **Start** seçin.
5. Başarılı kurulumdan sonra yalnızca `ux0:VPK/` içindeki kurulum dosyasını silebilirsiniz. Oyun ve kayıt klasörünü silmeyin.

## Kontroller

| Tuş | İşlev |
|---|---|
| Yön tuşları / sol analog | Hareket, yukarı nişan, çömelme |
| Cross | Zıplama / menü onayı |
| Square | Ateş; yakındaki piyadeye yakın dövüş |
| Circle veya R | El bombası |
| Triangle | Araca binme / inme |
| START | Duraklatma |
| Menülerde Circle | Geri |

Yukarı + Square ile yukarı, havadayken aşağı + Square ile aşağı ateş edilir. Oyunun tuşları VitaShell'in onay ayarından bağımsızdır. Arayüz ve diyaloglar İngilizcedir.

## Güncelleme ve kayıt

Oyunu kapatın. Önce `ux0:data/KiyiHurdasi/save.dat` dosyasını bilgisayara yedekleyin. Aynı Title ID (`KHYI00001`) ile yayımlanan yeni VPK'yı mevcut oyunun üzerine kurun; eski oyunu kaldırmanız gerekmiyor.

Kayıt, bölüm ilerlemesini ve ayarları saklar; çatışmanın tam ortasından devam ettiren anlık kayıt değildir. Kontrol noktaları devam eden oyun oturumunda kullanılır. Klasördeki eski `KiyiHurdasi` adı uyumluluk için korunur.

## Bir sorun olursa

- Kurulum hatasında boş alanı ve dosyanın SHA-256 değerini kontrol edin; gerekiyorsa VPK'yı yeniden indirin.
- Oyun görünmüyorsa yanlış depolamaya kopyalamadığınızdan ve kurulumu tamamladığınızdan emin olun.
- Eski ikon görünüyorsa oyunu kapatıp cihazı yeniden başlatın; kayıt dosyasını silmeyin.
- Açılış, ses, kontrol veya yavaşlama sorununu [Issues](https://github.com/onurkuru/ironcoast/issues) üzerinden bildirin. Vita modeli, firmware, sürüm numarası, bölüm ve varsa hata kodunu ekleyin.

**Test durumu:** Paket VitaSDK ile derlendi ve içeriği denetlendi. Fiziksel Vita üzerinde kurulum, LiveArea, ses, kontroller ve performans henüz doğrulanmadı. README'deki ekran görüntüleri oyunun geliştirme bilgisayarındaki gerçek çizicisinden alınmıştır. Yeni bina ağırlıklı harita tasarımı bu sürüme dahil değildir.

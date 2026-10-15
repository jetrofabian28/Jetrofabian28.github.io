# C++ Web Page & Template Engine

Template web modern, responsif, dan berkinerja tinggi yang ditenagai oleh C++. Proyek ini menyediakan 3 opsi penggunaan sesuai kebutuhan Anda:

1. **Standalone HTTP Server (`server.cpp`)**: Server HTTP mandiri tanpa runtime eksternal (Node.js/Python). Mendukung live endpoint REST API `/api/status`.
2. **Static HTML Generator (`server.exe --generate`)**: Program C++ yang me-render variabel ke file `rendered.html`.
3. **CGI Mode (`cgi.cpp`)**: Untuk digunakan di server web seperti Apache, Nginx, Lighttpd, atau uHTTPd (router/embedded device).

---

## 📁 Struktur File

- [`index.html`](index.html): Template halaman web modern (HTML5 + CSS responsif + dark-tech theme) yang memuat tag placeholder seperti `{{PAGE_TITLE}}`, `{{SERVER_STATUS}}`, dll.
- [`server.cpp`](server.cpp): Kode server HTTP & template engine C++ (menggunakan Winsock di Windows dan POSIX socket di Linux).
- [`cgi.cpp`](cgi.cpp): Script C++ untuk mode CGI standar.
- [`rendered.html`](rendered.html): Hasil ekspor HTML setelah di-render dengan data variabel C++.

---

## 🚀 Cara Menjalankan

### Opsi 1: Menjalankan C++ HTTP Server (Rekomendasi)
Kompilasi dan jalankan server lokal:

```powershell
# Kompilasi di Windows
g++ -O2 -std=c++17 server.cpp -lws2_32 -o server.exe

# Jalankan server
.\server.exe
```

Setelah server berjalan, buka browser di:
👉 **[http://localhost:8080](http://localhost:8080)**

Fitur di mode ini:
- Menyajikan template `index.html` dengan variabel dinamis (uptime, OS info, dsb).
- Endpoint REST API `/api/status` yang otomatis di-refresh tiap 3 detik oleh JavaScript.

---

### Opsi 2: Ekspor ke File HTML Statis
Jika hanya ingin menghasilkan file HTML hasil render tanpa menjalankan server terus-menerus:

```powershell
.\server.exe --generate
```
File hasil render akan tersimpan di `rendered.html`.

---

### Opsi 3: Kompilasi CGI (Untuk Router / Web Server Eksternal)
```powershell
g++ -O2 -std=c++17 cgi.cpp -o cgi.exe
```

---

## 🛠️ Cara Menambah Variabel Template Baru

1. Di file [`index.html`](index.html), tambahkan placeholder baru, misalnya: `{{NAMA_PENGGUNA}}`.
2. Di file [`server.cpp`](server.cpp) (bagian fungsi `getPageContent`), tambahkan nilainya ke dalam `map`:
   ```cpp
   vars["NAMA_PENGGUNA"] = "Budi";
   ```
3. Re-compile program `server.cpp`.

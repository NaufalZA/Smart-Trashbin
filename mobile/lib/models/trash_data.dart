
class TrashData {
  final int kategori;
  final int jarak;
  final String timestamp;

  TrashData({
    required this.kategori,
    required this.jarak,
    required this.timestamp,
  });

  factory TrashData.fromJson(Map<String, dynamic> json) {
    return TrashData(
      kategori: json['kategori'],
      jarak: json['jarak'],
      timestamp: json['timestamp'],
    );
  }
}

class TrashCount {
  final int kategori;
  final int jumlah;

  TrashCount({
    required this.kategori,
    required this.jumlah,
  });

  factory TrashCount.fromJson(Map<String, dynamic> json) {
    return TrashCount(
      kategori: json['kategori'],
      jumlah: json['jumlah'],
    );
  }
}
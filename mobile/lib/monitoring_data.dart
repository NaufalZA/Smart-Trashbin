import 'dart:convert';
import 'package:flutter/material.dart';
import 'package:http/http.dart' as http;
import 'package:intl/intl.dart';
import 'dashboard.dart'; // Import untuk dashboard.dart

void main() {
  runApp(MonitoringDataPage());
}

class MonitoringDataPage extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Smart Trash Bin',
      theme: ThemeData(
        primarySwatch: Colors.blue,
        visualDensity: VisualDensity.adaptivePlatformDensity,
      ),
      home: HomePage(),
    );
  }
}

class HomePage extends StatefulWidget {
  @override
  _HomePageState createState() => _HomePageState();
}

class _HomePageState extends State<HomePage> {
  List<dynamic> data = [];
  bool isLoading = true;
  List<String> categories = ['Harian', 'Bulanan', 'Tahunan', 'Custom'];
  int selectedCategoryIndex = 0;

  TextEditingController startDateController = TextEditingController();
  TextEditingController endDateController = TextEditingController();

  String getKategoriName(int kategoriId) {
    switch (kategoriId) {
      case 1:
        return 'Organik';
      case 2:
        return 'Anorganik';
      case 3:
        return 'B3';
      default:
        return 'Kategori Tidak Dikenal';
    }
  }

  Color getKategoriColor(int kategoriId) {
    switch (kategoriId) {
      case 1:
        return Colors.green.shade200;
      case 2:
        return Colors.blue.shade200;
      case 3:
        return Colors.red.shade200;
      default:
        return Colors.grey.shade200;
    }
  }

  @override
  void initState() {
    super.initState();
    fetchData();
  }

  Future<void> fetchData() async {
    String url;
    String selectedCategory = categories[selectedCategoryIndex].toLowerCase();

    if (selectedCategory == 'harian') {
      url = 'https://smart-trashbin-api.onrender.com/api/harian';
    } else if (selectedCategory == 'bulanan') {
      url = 'https://smart-trashbin-api.onrender.com/api/bulanan';
    } else if (selectedCategory == 'tahunan') {
      url = 'https://smart-trashbin-api.onrender.com/api/tahunan';
    } else if (selectedCategory == 'custom') {
      String start = startDateController.text;
      String end = endDateController.text;
      if (start.isEmpty || end.isEmpty) {
        setState(() {
          isLoading = false;
          data = [];
        });
        return;
      }
      url = 'https://smart-trashbin-api.onrender.com/api/range?start=$start&end=$end';
    } else {
      url = 'https://smart-trashbin-api.onrender.com/api/getdata';
    }

    try {
      final response = await http.get(Uri.parse(url));

      if (response.statusCode == 200) {
        setState(() {
          data = json.decode(response.body);
          isLoading = false;
        });
      } else {
        throw Exception('Failed to load data');
      }
    } catch (e) {
      setState(() {
        isLoading = false;
        data = [];
      });
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(
          content: Text("Gagal mengambil data: $e"),
          backgroundColor: Colors.red,
        ),
      );
    }
  }

  Widget _buildCategoryChip(String category, int index) {
    bool isSelected = selectedCategoryIndex == index;
    return GestureDetector(
      onTap: () {
        setState(() {
          selectedCategoryIndex = index;
          isLoading = true;
          fetchData();
        });
      },
      child: Container(
        padding: EdgeInsets.symmetric(horizontal: 16, vertical: 8),
        margin: EdgeInsets.symmetric(horizontal: 4),
        decoration: BoxDecoration(
          color: isSelected ? Color(0xFF4A90E2) : Colors.white.withOpacity(0.2),
          borderRadius: BorderRadius.circular(20),
          border: Border.all(
            color: isSelected ? Colors.transparent : Colors.white54,
            width: 1,
          ),
        ),
        child: Text(
          category,
          style: TextStyle(
            color: isSelected ? Colors.white : Colors.white70,
            fontWeight: isSelected ? FontWeight.bold : FontWeight.normal,
          ),
        ),
      ),
    );
  }

  Widget _buildDatePickers() {
    if (selectedCategoryIndex != 3) return SizedBox.shrink();

    return Padding(
      padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 12),
      child: Row(
        children: [
          Expanded(
            child: _buildDateField(startDateController, 'Mulai'),
          ),
          SizedBox(width: 10),
          Expanded(
            child: _buildDateField(endDateController, 'Akhir'),
          ),
          SizedBox(width: 10),
          ElevatedButton(
            onPressed: fetchData,
            style: ElevatedButton.styleFrom(
              backgroundColor: Color(0xFF50E3C2),
              shape: CircleBorder(),
              padding: EdgeInsets.all(12),
            ),
            child: Icon(Icons.search, color: Colors.white),
          ),
        ],
      ),
    );
  }

  Widget _buildDateField(TextEditingController controller, String label) {
    return TextField(
      controller: controller,
      decoration: InputDecoration(
        labelText: 'Tanggal $label',
        filled: true,
        fillColor: Colors.white.withOpacity(0.2),
        border: OutlineInputBorder(
          borderRadius: BorderRadius.circular(10),
          borderSide: BorderSide.none,
        ),
      ),
      readOnly: true,
      onTap: () async {
        DateTime? pickedDate = await showDatePicker(
          context: context,
          initialDate: DateTime.now(),
          firstDate: DateTime(2000),
          lastDate: DateTime(2101),
        );
        if (pickedDate != null) {
          controller.text = DateFormat('yyyy-MM-dd').format(pickedDate);
        }
      },
    );
  }

  Widget _buildDataList() {
    if (data.isEmpty) {
      return Center(
        child: Text(
          'Tidak ada data tersedia',
          style: TextStyle(color: Colors.white70, fontSize: 16),
        ),
      );
    }

    return ListView.builder(
      padding: EdgeInsets.symmetric(horizontal: 16, vertical: 8),
      itemCount: data.length,
      itemBuilder: (context, index) {
        var item = data[index];
        String kategoriName = getKategoriName(item['kategori']);
        Color categoryColor = getKategoriColor(item['kategori']);

        return Container(
          margin: EdgeInsets.symmetric(vertical: 8),
          decoration: BoxDecoration(
            color: Colors.white.withOpacity(0.1),
            borderRadius: BorderRadius.circular(15),
            border: Border.all(color: Colors.white38, width: 1),
          ),
          child: ListTile(
            contentPadding: EdgeInsets.symmetric(horizontal: 16, vertical: 12),
            leading: Container(
              padding: EdgeInsets.all(10),
              decoration: BoxDecoration(
                color: categoryColor.withOpacity(0.2),
                shape: BoxShape.circle,
              ),
              child: Icon(
                _getIconForKategori(item['kategori']),
                color: categoryColor,
                size: 24,
              ),
            ),
            title: Text(
              kategoriName,
              style: TextStyle(
                fontWeight: FontWeight.bold,
                color: Colors.white,
              ),
            ),
            subtitle: Text(
              'Jumlah Sampah: ${item['jumlah'] ?? 'Data tidak ada'}',
              style: TextStyle(color: Colors.white70),
            ),
            trailing: Icon(
              Icons.chevron_right,
              color: Colors.white70,
            ),
          ),
        );
      },
    );
  }

  IconData _getIconForKategori(int kategoriId) {
    switch (kategoriId) {
      case 1:
        return Icons.nature_people;
      case 2:
        return Icons.delete_sweep;
      case 3:
        return Icons.dangerous;
      default:
        return Icons.help_outline;
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        backgroundColor: Color(0xFF4A90E2),
        leading: IconButton(
          icon: Icon(Icons.arrow_back),
          onPressed: () {
            Navigator.push(
              context,
              MaterialPageRoute(builder: (context) => Dashboard()),
            );
          },
        ),
      ),
      body: Container(
        decoration: const BoxDecoration(
          gradient: LinearGradient(
            colors: [Color(0xFF4A90E2), Color(0xFF50E3C2)],
            begin: Alignment.topCenter,
            end: Alignment.bottomCenter,
          ),
        ),
        child: SafeArea(
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              Center(
                child: Padding(
                  padding: const EdgeInsets.symmetric(vertical: 16.0),
                  child: Text(
                    'Monitoring Sampah',
                    style: TextStyle(
                      color: Colors.white,
                      fontSize: 24,
                      fontWeight: FontWeight.bold,
                    ),
                  ),
                ),
              ),
              const Divider(
                color: Colors.white54,
                thickness: 1,
                indent: 16,
                endIndent: 16,
              ),
              SingleChildScrollView(
                scrollDirection: Axis.horizontal,
                padding: EdgeInsets.symmetric(horizontal: 16),
                child: Row(
                  children: List.generate(
                    categories.length,
                    (index) => _buildCategoryChip(categories[index], index),
                  ),
                ),
              ),
              _buildDatePickers(),
              Expanded(
                child: isLoading
                    ? Center(child: CircularProgressIndicator(color: Colors.white))
                    : _buildDataList(),
              ),
            ],
          ),
        ),
      ),
    );
  }
}

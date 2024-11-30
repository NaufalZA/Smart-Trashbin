import 'package:http/http.dart' as http;
import 'dart:convert';
import '../models/trash_data.dart';

class ApiService {
  static const String baseUrl = 'https://smart-trashbin.onrender.com/api';

  Future<List<TrashData>> getData() async {
    final response = await http.get(Uri.parse('$baseUrl/getdata'));
    if (response.statusCode == 200) {
      List jsonResponse = json.decode(response.body);
      return jsonResponse.map((data) => TrashData.fromJson(data)).toList();
    } else {
      throw Exception('Failed to load data');
    }
  }

  Future<List<TrashCount>> getPeriodData(String endpoint) async {
    final response = await http.get(Uri.parse('$baseUrl/$endpoint'));
    if (response.statusCode == 200) {
      List jsonResponse = json.decode(response.body);
      return jsonResponse.map((data) => TrashCount.fromJson(data)).toList();
    } else {
      throw Exception('Failed to load data');
    }
  }

  Future<List<TrashCount>> getRangeData(DateTime start, DateTime end) async {
    final response = await http.get(
      Uri.parse('$baseUrl/range?start=${start.toIso8601String()}&end=${end.toIso8601String()}')
    );
    if (response.statusCode == 200) {
      List jsonResponse = json.decode(response.body);
      return jsonResponse.map((data) => TrashCount.fromJson(data)).toList();
    } else {
      throw Exception('Failed to load data');
    }
  }
}
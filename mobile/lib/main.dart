import 'package:flutter/material.dart';
import 'services/api_service.dart';
import 'models/trash_data.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Smart Trashbin',
      theme: ThemeData(
        colorScheme: ColorScheme.fromSeed(seedColor: Colors.green),
        useMaterial3: true,
      ),
      home: const DashboardPage(),
    );
  }
}

class DashboardPage extends StatefulWidget {
  const DashboardPage({super.key});

  @override
  State<DashboardPage> createState() => _DashboardPageState();
}

class _DashboardPageState extends State<DashboardPage> {
  final ApiService _apiService = ApiService();

  @override
  Widget build(BuildContext context) {
    return DefaultTabController(
      length: 4,
      child: Scaffold(
        appBar: AppBar(
          title: const Text('Smart Trashbin Dashboard'),
          bottom: const TabBar(
            tabs: [
              Tab(text: 'Latest'),
              Tab(text: 'Daily'),
              Tab(text: 'Monthly'),
              Tab(text: 'Yearly'),
            ],
          ),
        ),
        body: TabBarView(
          children: [
            _buildLatestDataTab(),
            _buildPeriodDataTab('harian'),
            _buildPeriodDataTab('bulanan'),
            _buildPeriodDataTab('tahunan'),
          ],
        ),
      ),
    );
  }

  Widget _buildLatestDataTab() {
    return FutureBuilder<List<TrashData>>(
      future: _apiService.getData(),
      builder: (context, snapshot) {
        if (snapshot.connectionState == ConnectionState.waiting) {
          return const Center(child: CircularProgressIndicator());
        }
        if (snapshot.hasError) {
          return Center(child: Text('Error: ${snapshot.error}'));
        }
        if (!snapshot.hasData || snapshot.data!.isEmpty) {
          return const Center(child: Text('No data available'));
        }

        return ListView.builder(
          itemCount: snapshot.data!.length,
          itemBuilder: (context, index) {
            final data = snapshot.data![index];
            return ListTile(
              title: Text('Category ${data.kategori}'),
              subtitle: Text('Distance: ${data.jarak}cm\n${data.timestamp}'),
            );
          },
        );
      },
    );
  }

  Widget _buildPeriodDataTab(String period) {
    return FutureBuilder<List<TrashCount>>(
      future: _apiService.getPeriodData(period),
      builder: (context, snapshot) {
        if (snapshot.connectionState == ConnectionState.waiting) {
          return const Center(child: CircularProgressIndicator());
        }
        if (snapshot.hasError) {
          return Center(child: Text('Error: ${snapshot.error}'));
        }
        if (!snapshot.hasData || snapshot.data!.isEmpty) {
          return const Center(child: Text('No data available'));
        }

        return ListView.builder(
          itemCount: snapshot.data!.length,
          itemBuilder: (context, index) {
            final data = snapshot.data![index];
            return ListTile(
              title: Text('Category ${data.kategori}'),
              trailing: Text('Count: ${data.jumlah}'),
            );
          },
        );
      },
    );
  }
}

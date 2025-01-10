import 'package:flutter/material.dart';

class ProfilePage extends StatelessWidget {
  const ProfilePage({super.key});

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Profil Kelompok'),
        backgroundColor: const Color(0xFF4A90E2),
        centerTitle: true,
        elevation: 0,
      ),
      body: Container(
        decoration: const BoxDecoration(
          gradient: LinearGradient(
            colors: [Color(0xFF4A90E2), Color(0xFF50E3C2)],
            begin: Alignment.topCenter,
            end: Alignment.bottomCenter,
          ),
        ),
        child: SingleChildScrollView(
          child: Padding(
            padding: const EdgeInsets.all(20.0),
            child: Column(
              children: [
                // Logo atau Icon Kelompok
                const CircleAvatar(
                  radius: 60,
                  backgroundColor: Colors.white,
                  child: Icon(
                    Icons.groups,
                    size: 60,
                    color: Color(0xFF4A90E2),
                  ),
                ),
                const SizedBox(height: 24),
                
                // Nama Kelompok dalam Card
                Card(
                  elevation: 8,
                  shape: RoundedRectangleBorder(
                    borderRadius: BorderRadius.circular(15),
                  ),
                  color: Colors.white.withOpacity(0.9),
                  child: const Padding(
                    padding: EdgeInsets.all(16.0),
                    child: Column(
                      children: [
                        Text(
                          'KELOMPOK 1',
                          style: TextStyle(
                            fontSize: 24,
                            fontWeight: FontWeight.bold,
                            color: Color(0xFF4A90E2),
                          ),
                        ),
                        SizedBox(height: 8),
                        Text(
                          'Smart Trash Bin Project',
                          style: TextStyle(
                            fontSize: 16,
                            color: Colors.grey,
                          ),
                        ),
                      ],
                    ),
                  ),
                ),
                const SizedBox(height: 24),
                
                // Daftar Anggota dalam Cards
                const Text(
                  'Anggota Kelompok',
                  style: TextStyle(
                    fontSize: 22,
                    fontWeight: FontWeight.bold,
                    color: Colors.white,
                  ),
                ),
                const SizedBox(height: 16),
                
                // Card untuk setiap anggota
                _buildMemberCard(
                  'Naufal Zaidan',
                  'Anggota',
                  Icons.person,
                ),
                _buildMemberCard(
                  'Nur Aliya Al-Ghayashi',
                  'Anggota',
                  Icons.person,
                ),
                _buildMemberCard(
                  'Silvy Nur Azkia',
                  'Anggota',
                  Icons.person,
                ),
                _buildMemberCard(
                  'Mohamad Dedrick Finnegan',
                  'Anggota',
                  Icons.person,
                ),
                _buildMemberCard(
                  'Oki Ariansyah',
                  'Anggota',
                  Icons.person,
                ),
                _buildMemberCard(
                  'Devit Saputra',
                  'Anggota',
                  Icons.person,
                ),
                _buildMemberCard(
                  'Daffa Faris Abqari Ramdhani',
                  'Anggota',
                  Icons.person,
                ),
                _buildMemberCard(
                  'Abel Wirawan Suzanto',
                  'Anggota',
                  Icons.person,
                ),
              ],
            ),
          ),
        ),
      ),
    );
  }

  Widget _buildMemberCard(String name, String role, IconData icon) {
    return Container(
      margin: const EdgeInsets.only(bottom: 12),
      child: Card(
        elevation: 4,
        shape: RoundedRectangleBorder(
          borderRadius: BorderRadius.circular(12),
        ),
        color: Colors.white.withOpacity(0.9),
        child: ListTile(
          contentPadding: const EdgeInsets.symmetric(
            horizontal: 16,
            vertical: 8,
          ),
          leading: CircleAvatar(
            backgroundColor: const Color(0xFF4A90E2),
            child: Icon(
              icon,
              color: Colors.white,
            ),
          ),
          title: Text(
            name,
            style: const TextStyle(
              fontWeight: FontWeight.bold,
              fontSize: 16,
            ),
          ),
          subtitle: Text(
            role,
            style: TextStyle(
              color: Colors.grey[600],
            ),
          ),
        ),
      ),
    );
  }
}

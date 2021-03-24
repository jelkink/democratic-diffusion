#ifndef _CITIZEN_H
#define _CITIZEN_H

class CProvince;
class CModel;

class CCitizen
{
 public:
  CCitizen(CModel* p_pModel, CProvince* p_pProvince);
  virtual ~CCitizen();

  void DetermineCommunication();
  void Communicate(CCitizen* p_pCitizen);
  void ReceiveBroadcast(bool p_bDemocracy);
  void DetermineProtest();
  void SetProtesting(bool p_bProtesting);

  bool IsProtesting() { return m_bProtesting; }
  int GetAttitude() { return m_nAttitude; }
  int GetInGroupThreshold() { return m_nInGroupThreshold; }
  int GetOutGroupThreshold() { return m_nOutGroupThreshold; }
  CProvince* GetProvince() { return m_pProvince; }

  void SetAttitude(int p_nAttitude) { m_nAttitude = p_nAttitude; }
  void SetInGroupThreshold(int p_nInGroupThreshold);
  void SetOutGroupThreshold(int p_nOutGroupThreshold);
  void SetProvince(CProvince& p_Province) { m_pProvince = &p_Province; }

 private:
  bool m_bProtesting;
  int m_nAttitude;
  int m_nInGroupThreshold, m_nOutGroupThreshold;
  CProvince* m_pProvince;
  CModel* m_pModel;
};

#endif

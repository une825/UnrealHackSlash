using System;
using System.Collections.Generic;
using System.Linq;

namespace MyHackSlash.Core
{
    #region Core Enumerations & Types

    public enum HEGemCategory { Main, Support }
    public enum HEGemType { Common, Melee, Projectile }
    public enum HEElement { Physical, Fire, Ice, Lightning, Magic }
    public enum EHWaveType { Battle, Boss, Reward, Shop }
    public enum EHWaveClearType { TimeSurvival, KillCount, ManualUI }

    /// <summary>
    /// 보조젬이 제공하는 효과의 종류
    /// </summary>
    public enum HESupportEffectType
    {
        DamageBoost,        // 데미지 증폭 (%)
        ProjectileCount,    // 투사체 개수 추가 (+)
        AttackSpeed,        // 공격 속도 증가 (%)
        AreaOfEffect,       // 공격 범위 확대 (%)
        ChainCount          // 연쇄 횟수 추가 (+)
    }

    #endregion

    #region Data Models & Stats

    /// <summary>
    /// 스킬의 최종 성능을 나타내는 스탯 구조체
    /// </summary>
    public struct FSkillStats
    {
        public float BaseDamage;
        public int ProjectileCount;
        public float AttackSpeed;
        public float Range;
        public int ChainCount;

        public static FSkillStats Default() => new FSkillStats 
        { 
            BaseDamage = 10f, ProjectileCount = 1, AttackSpeed = 1.0f, Range = 100f, ChainCount = 0 
        };

        public override string ToString() => 
            $"Damage: {BaseDamage:F1}, Projectiles: {ProjectileCount}, Speed: {AttackSpeed:F1}, Chain: {ChainCount}";
    }

    /// <summary>
    /// 젬의 데이터 정의 (보조젬 전용 데이터 필드 추가)
    /// </summary>
    public record FHGemData(
        string GemId,
        string Name,
        HEGemCategory Category,
        HEGemType TargetType, // 메인젬이면 자신의 타입, 보조젬이면 장착 가능한 대상 타입
        HEElement Element,
        int Tier,
        string NextTierGemId,
        HESupportEffectType? SupportEffect = null,
        float EffectValue = 0f
    );

    #endregion

    #region Skill Gem System (Inventory, Upgrade & Socketing)

    /// <summary>
    /// 실제 사용자가 장착하여 활성화된 스킬 인스턴스
    /// </summary>
    public class HActiveSkill
    {
        public FHGemData MainGem { get; }
        private readonly List<FHGemData> _linkedSupports = new();
        public IReadOnlyList<FHGemData> LinkedSupports => _linkedSupports;

        public HActiveSkill(FHGemData mainGem)
        {
            if (mainGem.Category != HEGemCategory.Main)
                throw new ArgumentException("ActiveSkill must start with a Main Gem.");
            MainGem = mainGem;
        }

        /// <summary>
        /// 보조젬을 장착합니다. 호환성 검사 로직 포함.
        /// </summary>
        public bool LinkSupportGem(FHGemData supportGem)
        {
            if (supportGem.Category != HEGemCategory.Support) return false;

            // 호환성 체크: 보조젬의 TargetType이 Common이거나 메인젬의 타입과 일치해야 함
            bool isCompatible = supportGem.TargetType == HEGemType.Common || 
                               supportGem.TargetType == MainGem.TargetType;

            if (!isCompatible)
            {
                Console.WriteLine($"[Skill] Incompatible: {supportGem.Name} cannot support {MainGem.Name}");
                return false;
            }

            _linkedSupports.Add(supportGem);
            Console.WriteLine($"[Skill] Linked {supportGem.Name} to {MainGem.Name}");
            return true;
        }

        /// <summary>
        /// 장착된 모든 보조젬의 효과를 적용하여 최종 스탯을 계산합니다. (Business Logic)
        /// </summary>
        public FSkillStats GetFinalStats()
        {
            FSkillStats stats = FSkillStats.Default();

            foreach (var support in _linkedSupports)
            {
                if (!support.SupportEffect.HasValue) continue;

                switch (support.SupportEffect.Value)
                {
                    case HESupportEffectType.DamageBoost:
                        stats.BaseDamage *= (1.0f + support.EffectValue);
                        break;
                    case HESupportEffectType.ProjectileCount:
                        stats.ProjectileCount += (int)support.EffectValue;
                        break;
                    case HESupportEffectType.AttackSpeed:
                        stats.AttackSpeed += support.EffectValue;
                        break;
                    case HESupportEffectType.ChainCount:
                        stats.ChainCount += (int)support.EffectValue;
                        break;
                }
            }

            return stats;
        }
    }

    /// <summary>
    /// 스킬 젬 인벤토리 및 합성 시스템
    /// </summary>
    public class HGemInventorySystem
    {
        private readonly List<FHGemData> _inventory = new();
        private readonly Dictionary<string, FHGemData> _gemDatabase;

        public event Action<FHGemData> OnGemAdded;
        public event Action<FHGemData> OnGemUpgraded;

        public HGemInventorySystem(Dictionary<string, FHGemData> database)
        {
            _gemDatabase = database;
        }

        public void AddGem(string gemId)
        {
            if (!_gemDatabase.TryGetValue(gemId, out var gemData)) return;

            _inventory.Add(gemData);
            OnGemAdded?.Invoke(gemData);
            CheckAndUpgrade(gemId);
        }

        private void CheckAndUpgrade(string gemId)
        {
            var sameGems = _inventory.Where(g => g.GemId == gemId).ToList();
            if (sameGems.Count >= 3)
            {
                var baseGem = sameGems.First();
                if (string.IsNullOrEmpty(baseGem.NextTierGemId)) return;

                for (int i = 0; i < 3; i++) _inventory.Remove(sameGems[i]);

                Console.WriteLine($"[System] Upgrade: {baseGem.Name} x3 -> Tier UP!");
                AddGem(baseGem.NextTierGemId);
                OnGemUpgraded?.Invoke(baseGem);
            }
        }

        public FHGemData GetGemFromInventory(string gemId) => _inventory.FirstOrDefault(g => g.GemId == gemId);
    }

    #endregion

    #region Wave & Economy System

    public class HWaveManager
    {
        public enum EHWaveState { Ready, InProgress, Completed }
        public EHWaveState CurrentState { get; private set; } = EHWaveState.Ready;

        private int _currentWaveIndex = 0;
        private float _currentProgressValue = 0f;
        private FHWaveData _currentWaveData;
        
        private int _playerGold = 0;
        private const float InterestRate = 0.1f;
        private const int MaxInterest = 50;

        public event Action<int, EHWaveType> OnWaveStarted;
        public event Action<float> OnProgressUpdated;
        public event Action<int, int> OnWaveCompleted;

        private readonly List<FHWaveData> _waveConfigs;

        public HWaveManager(List<FHWaveData> configs)
        {
            _waveConfigs = configs;
        }

        public void StartWave(int index)
        {
            if (index < 0 || index >= _waveConfigs.Count) return;
            _currentWaveIndex = index;
            _currentWaveData = _waveConfigs[index];
            _currentProgressValue = 0f;
            CurrentState = EHWaveState.InProgress;
            OnWaveStarted?.Invoke(_currentWaveIndex + 1, _currentWaveData.WaveType);
        }

        public void UpdateProgress(float value)
        {
            if (CurrentState != EHWaveState.InProgress) return;
            _currentProgressValue += value;
            float percent = Math.Clamp(_currentProgressValue / _currentWaveData.ClearValue, 0f, 1f);
            OnProgressUpdated?.Invoke(percent);
            if (percent >= 1.0f) CompleteWave();
        }

        private void CompleteWave()
        {
            CurrentState = EHWaveState.Completed;
            int interest = (int)Math.Min(_playerGold * InterestRate, MaxInterest);
            _playerGold += interest;
            OnWaveCompleted?.Invoke(_currentWaveIndex + 1, interest);
        }

        public void AddGold(int amount) => _playerGold += amount;
    }

    public class FHWaveData
    {
        public int WaveIndex { get; set; }
        public EHWaveType WaveType { get; set; }
        public EHWaveClearType ClearType { get; set; }
        public float ClearValue { get; set; }
    }

    #endregion

    #region Simulation & Integration

    public class HackSlashGameEngine
    {
        private readonly HGemInventorySystem _inventorySystem;
        private readonly HWaveManager _waveManager;
        private readonly Dictionary<string, FHGemData> _db;

        public HackSlashGameEngine()
        {
            // 젬 데이터베이스 확장: 메인젬과 보조젬 정의
            _db = new Dictionary<string, FHGemData>
            {
                // Main Gems
                { "Fireball", new FHGemData("Fireball", "Fireball", HEGemCategory.Main, HEGemType.Projectile, HEElement.Fire, 1, "Fireball2") },
                { "HeavyStrike", new FHGemData("HeavyStrike", "Heavy Strike", HEGemCategory.Main, HEGemType.Melee, HEElement.Physical, 1, null) },
                
                // Support Gems
                { "Multicast", new FHGemData("Multicast", "Multiple Projectiles", HEGemCategory.Support, HEGemType.Projectile, HEElement.Magic, 1, null, HESupportEffectType.ProjectileCount, 2f) },
                { "Sharpness", new FHGemData("Sharpness", "Physical Sharpness", HEGemCategory.Support, HEGemType.Melee, HEElement.Physical, 1, null, HESupportEffectType.DamageBoost, 0.3f) },
                { "Haste", new FHGemData("Haste", "Common Haste", HEGemCategory.Support, HEGemType.Common, HEElement.Magic, 1, null, HESupportEffectType.AttackSpeed, 0.2f) }
            };

            _inventorySystem = new HGemInventorySystem(_db);
            _waveManager = new HWaveManager(new List<FHWaveData> { new FHWaveData { WaveIndex = 1, WaveType = EHWaveType.Battle, ClearValue = 5 } });
        }

        public void RunSimulation()
        {
            Console.WriteLine("=== HackSlash Advanced Gem System Simulation ===\n");

            // 1. 스킬 인스턴스 생성 (파이어볼)
            var fireballSkill = new HActiveSkill(_db["Fireball"]);
            Console.WriteLine($"Initial Stats: {fireballSkill.GetFinalStats()}");

            // 2. 보조젬 장착 시도
            Console.WriteLine("\n[Scenario: Socketing Support Gems]");
            
            // 호환되는 보조젬 장착 (Projectile 전용)
            fireballSkill.LinkSupportGem(_db["Multicast"]);
            
            // 공통 보조젬 장착 (Common)
            fireballSkill.LinkSupportGem(_db["Haste"]);

            // 호환되지 않는 보조젬 장착 시도 (Melee 전용을 Projectile에)
            fireballSkill.LinkSupportGem(_db["Sharpness"]);

            // 3. 최종 결과 확인
            Console.WriteLine($"\nFinal Modified Stats: {fireballSkill.GetFinalStats()}");

            // 4. 웨이브 및 이자 시스템 가동
            Console.WriteLine("\n[Scenario: Wave Economy]");
            _waveManager.AddGold(100);
            _waveManager.StartWave(0);
            _waveManager.UpdateProgress(5f); // 웨이브 클리어
            
            _waveManager.OnWaveCompleted += (idx, interest) => 
                Console.WriteLine($"Wave {idx} Cleared! Interest: {interest} Gold. Player Gold: 110 (Expected)");
        }
    }

    #endregion
}

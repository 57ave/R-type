### R-Type Network Snapshot Optimization – Delta Compression and Protocol refining

**Before**  
Sent: 13 034 B/s
Received: 1 080 B/s
TotalSent (measured): 616 826 B 
TotalRecv: 99 858 B

**After**  
Sent: **8 774 B/s**  
Received: 1 071 B/s  
TotalSent (measured): 171 640 B  
TotalRecv: 97 785 B

**Key Improvements**
- **Outbound bandwidth**: 13 034 → **8 774 B/s**  
  → **–4 260 B/s** saved  
  → **–32.7%** reduction (sustained rate)
- Cumulative sent bytes (measured interval): **–72.2%** (616 kB → 172 kB)  
  (Note: absolute TotalSent difference affected by varying run durations)
- Inbound traffic: essentially unchanged (–0.8%)

**What Changed (High-Level)**  
Replaced full world snapshot broadcast with **delta snapshot system**:

- Per-room + global cache of last-sent EntityState
- Only send entities that are **new** or **changed**
- (position delta > 0.05, velocity Δ > 0.01, or any integer field changed)
- Skip sending packet to a room if **nothing changed**
- Force full snapshot at GAME_START


**Quick Math**  
~4 260 B/s saved ÷ 30 Hz ≈ **142 bytes less per snapshot** on average  
→ Roughly 3–6 fewer full EntityStates sent per tick (depending on serialization size)


**Status**  
✅ Implemented & measured  
✅ ~33% sustained outbound bandwidth reduction  
✅ No meaningful regression in received traffic

**Verdict**  
Solid, high-leverage optimisation with very good return for the complexity added.  
Adding **sequence numbers + resync** and **on-join full snapshot** will make it production-ready.
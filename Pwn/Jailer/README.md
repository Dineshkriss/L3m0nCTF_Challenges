# 🔒 Jailer

> *"They built the perfect prison. Impenetrable walls. No doors. No windows. But they forgot one thing... some things slip through the bars."*

---

## 📋 Challenge Info

| Field | Value |
|-------|-------|
| **Category** | Reverse Engineering |
| **Difficulty** | ⭐⭐⭐ Medium |
| **Points** | 300 |
| **Author** | L3m0n Team |

---

## 📖 Story

The warden boasts that his digital prison is escape-proof. Every inmate is thrown into solitary confinement — a void where nothing exists, and nothing can be accessed.

**"You can scream all you want,"** he laughs. **"There's nothing here to hear you."**

But rumors whisper of a flaw in the system. Something the architects overlooked. Something that was there *before* the walls went up.

Can you find what they left behind?

---

## 🎯 Objective

You're inside the jail. The flag exists somewhere in this system, but you can't reach it through conventional means.

Think like an escape artist. What still exists even after the prison doors slam shut?

---

## 🚀 Connection

```bash
nc <CHALLENGE_IP> 1338
```

---

## 📁 Files

- `jailer` - The prison binary

---

## 🏁 Flag Format

```
L3m0nCTF{...}
```

---

## 💡 Hints

<details>
<summary>Hint 1 (Free)</summary>
The binary creates a sandbox. But sandboxes have boundaries, not perfect isolation.
</details>

<details>
<summary>Hint 2 (-50 points)</summary>
What happens to resources opened before entering a restricted environment?
</details>

<details>
<summary>Hint 3 (-100 points)</summary>
Numbers between 100-200 might be interesting...
</details>

---

*Good luck, prisoner. 🗝️*

# UKF + ECM Battery Estimation Algorithm — Theory of Operation

This document explains the **combined Unscented Kalman Filter (UKF)** and  
**Equivalent Circuit Model (ECM)** battery estimation algorithm implemented in:

- `ukf.c / ukf.h` — generic Unscented Kalman Filter
- `ecm.c / ecm.h` — 20-point table ECM with Arrhenius temperature scaling
- `ukf_ecm_test.c` — integrated unit test

All LaTeX follows GitHub Markdown rules:
- **Inline math:** `$ ... $`
- **Block math:** `$$ ... $$`

---

## 1. Problem Statement

We seek to estimate internal battery states that are **not directly measurable**, using only:

- Terminal voltage $V$
- Current $I$
- Ambient temperature $T_{amb}$

The internal states include:

- State of Charge ($SOC$)
- RC diffusion voltage ($V_{RC}$)
- Voltage hysteresis ($H$)
- Core cell temperature ($T$)

The algorithm combines:

1. A **physics-based ECM** to describe battery behavior
2. A **UKF** to estimate states and reject noise
3. Online adaptation of ECM parameters ($R_0$, $R_1$, $C_1$, hysteresis)

---

## 2. State Vector and Inputs

### 2.1 UKF State Vector

The UKF state is defined as:

$$
\mathbf{x} =
\begin{bmatrix}
SOC \\
V_{RC} \\
H \\
T
\end{bmatrix}
$$

Where:

- $SOC \in [0,1]$
- $V_{RC}$ is the diffusion RC voltage
- $H$ is voltage hysteresis
- $T$ is core cell temperature in $\ ^\circ\mathrm{C}$

### 2.2 Input Vector

The process input is:

$$
\mathbf{u} =
\begin{bmatrix}
I \\
T_{amb}
\end{bmatrix}
$$

with the convention:

- $I > 0$ → discharge  
- $I < 0$ → charge  

---

## 3. ECM Measurement Model

The ECM predicts terminal voltage as:

$$
V = OCV(SOC)
  + H_{dir}(SOC)
  + V_{RC}
  - I \, R_0(SOC, T)
$$

Where:

- $OCV(SOC)$ is obtained from a 20-point lookup table
- $H_{dir}(SOC)$ is selected from:
  - $H_{dsg}(SOC)$ if $I > 0$
  - $H_{chg}(SOC)$ if $I < 0$
- $R_0(SOC,T)$ is temperature-scaled ohmic resistance

This equation is used as the **UKF measurement function** $h(\mathbf{x})$.

---

## 4. ECM State Dynamics (Process Model)

### 4.1 SOC Dynamics

Battery capacity in Coulombs:

$$
Q_C = 3600 \cdot Q_{Ah}
$$

SOC evolution:

$$
\frac{d(SOC)}{dt} = -\frac{I}{Q_C}
$$

Discrete update:

$$
SOC_{k+1} = SOC_k - \frac{\Delta t}{Q_C} I_k
$$

---

### 4.2 RC Branch Dynamics

The RC diffusion branch is modeled as:

$$
\frac{dV_{RC}}{dt}
= -\frac{1}{R_1 C_1} V_{RC}
+ \frac{1}{C_1} I
$$

Discrete update:

$$
V_{RC,k+1} =
V_{RC,k}
+ \Delta t
\left(
-\frac{V_{RC,k}}{R_1 C_1}
+ \frac{I_k}{C_1}
\right)
$$

Where:

- $R_1(SOC,T)$ and $C_1(SOC,T)$ come from tables + Arrhenius scaling

---

### 4.3 Hysteresis Dynamics

Hysteresis is modeled as a **first-order lag** toward a current-dependent limit:

$$
\frac{dH}{dt} = \frac{H_\infty(SOC, I) - H}{\tau_H}
$$

Where:

$$
H_\infty =
\begin{cases}
H_{dsg}(SOC), & I > 0 \\
H_{chg}(SOC), & I < 0 \\
H,            & I \approx 0
\end{cases}
$$

Discrete update:

$$
H_{k+1} = H_k
+ \Delta t \frac{H_\infty - H_k}{\tau_H}
$$

---

### 4.4 Thermal Dynamics

A lumped thermal model is used:

$$
\frac{dT}{dt}
= \frac{1}{C_{th}}
\left(
I^2 R_0(SOC,T)
- \frac{T - T_{amb}}{R_{th}}
\right)
$$

Discrete update:

$$
T_{k+1}
= T_k
+ \Delta t
\frac{
I_k^2 R_0(SOC_k,T_k)
- \frac{T_k - T_{amb}}{R_{th}}
}{C_{th}}
$$

---

## 5. Arrhenius Temperature Scaling

The tables for $R_0$, $R_1$, and $C_1$ are defined at a reference temperature $T_{ref} = 20^\circ\mathrm{C}$.

Each parameter is scaled using:

$$
k(T) = k_{ref}
\exp\left(
-\frac{E_a}{R_g}
\left(
\frac{1}{T_K} - \frac{1}{T_{ref,K}}
\right)
\right)
$$

Where:

- $E_a$ is activation energy (J/mol)
- $R_g = 8.314\ \mathrm{J/(mol\cdot K)}$
- $T_K = T + 273.15$

Applied independently to:

- $R_0(SOC,T)$
- $R_1(SOC,T)$
- $C_1(SOC,T)$

---

## 6. Unscented Kalman Filter (UKF)

### 6.1 Sigma Points

For a state dimension $n$, UKF generates $2n+1$ sigma points:

$$
\lambda = \alpha^2 (n + \kappa) - n
$$

$$
\chi_0 = \mathbf{x}
$$

$$
\chi_i = \mathbf{x} + \sqrt{n+\lambda}\, \mathbf{L}_i
$$

$$
\chi_{i+n} = \mathbf{x} - \sqrt{n+\lambda}\, \mathbf{L}_i
$$

where $\mathbf{L}$ is the Cholesky factor of covariance $P$.

---

### 6.2 Prediction Step

Sigma points are propagated through the ECM process model:

$$
\chi_{k+1|k}^i = f(\chi_k^i, \mathbf{u}_k)
$$

Predicted mean:

$$
\hat{\mathbf{x}}_{k+1|k}
= \sum_i W_i^{(m)} \chi_{k+1|k}^i
$$

Predicted covariance:

$$
P_{k+1|k}
= \sum_i W_i^{(c)}
(\chi_{k+1|k}^i - \hat{\mathbf{x}}_{k+1|k})
(\cdot)^T
+ Q
$$

---

### 6.3 Measurement Update

Sigma points are transformed to voltage space:

$$
z_k^i = h(\chi_{k+1|k}^i)
$$

Predicted voltage:

$$
\hat{z}_k = \sum_i W_i^{(m)} z_k^i
$$

Innovation covariance:

$$
S = \sum_i W_i^{(c)} (z_k^i - \hat{z}_k)^2 + R
$$

Cross-covariance:

$$
P_{xz} = \sum_i W_i^{(c)}
(\chi_{k+1|k}^i - \hat{\mathbf{x}}_{k+1|k})
(z_k^i - \hat{z}_k)
$$

Kalman gain:

$$
K = P_{xz} S^{-1}
$$

State update:

$$
\mathbf{x}_{k+1}
= \hat{\mathbf{x}}_{k+1|k}
+ K (z_k - \hat{z}_k)
$$

Covariance update:

$$
P_{k+1} = P_{k+1|k} - K S K^T
$$

---

## 7. Online Parameter Identification

### 7.1 Ohmic Resistance $R_0$ from Rest Entry

At a current step to rest:

$$
R_0 \approx -\frac{\Delta V}{\Delta I}
$$

Table update (nearest SOC bin):

$$
R_0^{new}
= (1-\alpha) R_0^{old}
+ \alpha R_0^{est}
$$

---

### 7.2 $R_1$ and $C_1$ from RC Decay

During rest:

$$
V_{RC}(t) = V_{RC}(0^+) e^{-t/\tau}
$$

Log-linear form:

$$
\ln V_{RC}(t) = a - \frac{t}{\tau}
$$

From LSQ fit:

$$
\tau = -\frac{1}{b}
$$

Then:

$$
R_1 \approx \frac{V_{RC}(0^+)}{|I_{step}|}
$$

$$
C_1 \approx \frac{\tau}{R_1}
$$

---

## 8. Algorithm Summary

At each time step:

1. **True cell** generates $V_{true}$
2. Measurement noise → $V_{meas}$
3. UKF **predict** using ECM process model
4. UKF **update** using ECM voltage model
5. ECM tables optionally updated using UKF states
6. Output SOC, $T$, voltage, and parameters

---

## 9. Practical Implications

This architecture:

- Separates **physics** (ECM) from **estimation** (UKF)
- Supports:
  - SOC estimation
  - Temperature estimation
  - Hysteresis modeling
  - Online impedance learning
- Is suitable for:
  - Embedded BMS firmware
  - Silicon-anode and LFP chemistries
  - Field adaptation and aging tracking

---

**This README mathematically describes exactly what `ukf_ecm_test.c` executes.**


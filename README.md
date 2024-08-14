# JUCE-AudioPlugin-Synthesizer
An audio plugin build by JUCE, synth with Reverb, Modulation, Unison Effect, LFO Control, Filter,etc

使用C++ JUCE框架开发的音频插件

#UI端可控制的对象如下图：
![image](https://github.com/user-attachments/assets/7d699076-459e-4e10-8820-3b1f10b8ee6f)
![image](https://github.com/user-attachments/assets/0683b430-8abc-4321-a75b-bffd70303c36)
![image](https://github.com/user-attachments/assets/89a5b216-0751-4bd6-b98c-36c17714b52e)



音频DSP实现：
● 振荡器的基础类；再利用variadic class，std::variant实现其子类变体LFO；
● 混响Reverb效果；
● Unison效果与Detune效果；
● 设置LFO控制任何其他参数；
● 滤波器类型截止频率及Resonance频率；
● 合成器中主振荡器的ADSR

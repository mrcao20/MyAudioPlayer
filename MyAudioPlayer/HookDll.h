/**
	Ctrl+ALT+F8�ѱ�ʹ��Ϊж�ع���
*/

#pragma once

#include <functional>
#include <vector>

// ÿ�ε���SetHook֮ǰ�������Զ�����UnsetHook
void SetHook(bool isLocked = false);
// ���Ӵ��ڽ���ж�ع��ӣ��������ڣ���ʲô��Ҳ����
void UnsetHook();

/**
�������°�����̬������ʵ��Ϊ˫��forѭ�������Ծ���������ܵİ�������vector�ĵ�һλ�����������ڶ��ذ�������forѭ����ֻ���е�һ�ذ����������
��forѭ��
ex: ϣ�������İ������ΪCtrl+A����Ctrl����vector�ĵ�һλ��A���ڵڶ�λ��������Ctrl��δ������ʱ���ڶ���forѭ����Զ������Ч���Խ�ʡʱ��
*/
void SetFunction(const std::vector<int> &vk_key, const std::function<void()> &callback);
void SetFunctions(const std::vector<std::vector<int>> &vk_key, const std::vector<std::function<void()>> &callback);

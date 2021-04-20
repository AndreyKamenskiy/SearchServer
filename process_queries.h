#pragma once
#include "search_server.h"
#include <vector>

#include <iostream>

std::vector<std::vector<Document>> ProcessQueries(
    const SearchServer& search_server,
    const std::vector<std::string>& queries);

class JoinedDocuments {

    class BasicIterator {
        friend class JoinedDocuments;

    public:
        // ��������� ��������� - forward iterator
        // (��������, ������� ������������ �������� ���������� � ������������ �������������)
        using iterator_category = std::forward_iterator_tag;
        // ��� ���������, �� ������� ������������ ��������
        using value_type = Document;
        // ���, ������������ ��� �������� �������� ����� �����������
        using difference_type = std::ptrdiff_t;
        // ��� ��������� �� ����������� ��������
        using pointer = Document*;
        // ��� ������ �� ����������� ��������
        using reference = Document&;

        BasicIterator() {
            documents_ = nullptr;
            vectorNum_ = -1;
            documentNum_ = -1;
        }

        BasicIterator(std::vector<std::vector<Document>>* documents, size_t vectorNum, size_t documentNum)
            : vectorNum_(vectorNum),
            documentNum_(documentNum)
        {
            documents_ = documents;
            checkNotEmptyVector();
        }

        // �������������� �����������/����������� �����������
        BasicIterator(const BasicIterator& other) noexcept
            : 
            vectorNum_(other.vectorNum_),
            documentNum_(other.documentNum_)
        {
            documents_ = other.documents_;
            checkNotEmptyVector();
        }

        // ����� ���������� �� ������� �������������� �� ���������� ��������� = ��� �������
        // ����������������� ������������ �����������, ���� ������� �������� = �
        // �������� ���������� ������������� ��� �� ���.
        BasicIterator& operator=(const BasicIterator& rhs) 
        {
            documents_ = rhs.documents_;
            vectorNum_ = rhs.vectorNum_;
            documentNum_ = rhs.documentNum_;
            return *this;
        }   

        // �������� ��������� ���������� (� ���� ������� ��������� ��������� ����������� ��������)
        // ��� ��������� �����, ���� ��� ��������� �� ���� � ��� �� ������� ������, ���� �� end()
        [[nodiscard]] bool operator==(const BasicIterator& rhs) const noexcept {
            return documents_ == rhs.documents_ && vectorNum_ == rhs.vectorNum_ && documentNum_ == rhs.documentNum_;
        }

        // �������� �������� ���������� �� �����������
        // �������������� !=
        [[nodiscard]] bool operator!=(const BasicIterator& rhs) const noexcept {
            return !(*this == rhs);
        }

        // �������� ��������������. ����� ��� ������ �������� ��������� �� ��������� ������� ������
        // ���������� ������ �� ������ ����
        // ��������� ���������, �� ������������ �� ������������ ������� ������, �������� � �������������� ���������
        BasicIterator& operator++() noexcept {
            findNextNotEmpty();
            return *this;
        }

        // �������� ��������������. ����� ��� ������ �������� ��������� �� ��������� ������� ������.
        // ���������� ������� �������� ���������
        // ��������� ���������, �� ������������ �� ������������ ������� ������,
        // �������� � �������������� ���������
        BasicIterator operator++(int) noexcept {
            BasicIterator old_value(*this);
            ++(*this);
            return old_value;
        }

        // �������� �������������. ���������� ������ �� ������� �������
        // ����� ����� ��������� � ���������, �� ������������ �� ������������ ������� ������,
        // �������� � �������������� ���������
        [[nodiscard]] reference operator*() const noexcept {
            return documents_->at(vectorNum_).at(documentNum_);
        }

        // �������� ������� � ����� ������. ���������� ��������� �� ������� ������� ������.
        // ����� ����� ��������� � ���������, �� ������������ �� ������������ ������� ������,
        // �������� � �������������� ���������
        [[nodiscard]] pointer operator->() const noexcept {
            return &(documents_->at(vectorNum_).at(documentNum_));
        }

    private:
        size_t vectorNum_ = 0;
        size_t documentNum_ = 0;
        std::vector<std::vector<Document>>* documents_;

        void findNextNotEmpty() {
            if (documentNum_ + 1 < (*documents_)[vectorNum_].size()) {
                ++documentNum_;
            }
            else {
                vectorNum_++;
                //find first not empty vector
                while (vectorNum_ < documents_->size() && documents_->at(vectorNum_).empty()) { 
                    ++vectorNum_; 
                }
                //if not find
                if (vectorNum_ == documents_->size()) {
                    *this = BasicIterator();
                }
                else {
                    documentNum_ = 0;
                }
            }
        }

        void checkNotEmptyVector() {
            if ((*documents_)[vectorNum_].empty()) {
                findNextNotEmpty();
            }
        }
    };

public:
    using value_type = Document;
    using reference = value_type&;
    using const_reference = const value_type&;

    using Iterator = BasicIterator;
    using ConstIterator = const BasicIterator;


    explicit JoinedDocuments(std::vector<std::vector<Document>>&& documents)
        :documents_(documents)  {
    }
    
    Iterator begin() noexcept {
        return Iterator(&documents_, 0, 0);
    }

    Iterator end() {
        return Iterator();
    }

private:
    std::vector<std::vector<Document>> documents_;
};

JoinedDocuments ProcessQueriesJoined(
    const SearchServer& search_server,
    const std::vector<std::string>& queries);
